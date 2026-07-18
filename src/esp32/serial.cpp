#include <map>
#include <string>
#include <thread>
#include <vector>

#include "core/log.hpp"
#include "esp32/serial.hpp"
#include "serial/serial.h"

using namespace std::chrono_literals;

namespace mbr {

// Initializes and maintains Serial behavior
void serial_manager_t::start() {
    if (worker_.joinable()) { return; }
    keep_running = true;
    worker_      = std::thread([this]() {
        while (keep_running) {
            this->clean_ports();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (is_serial_write) { this->receive_data(); }
        }
    });
}
// Opens a port
bool serial_manager_t::open_port(const std::string& port, const std::string& description) {
    try {
        serial::Timeout timeout;
        timeout.inter_byte_timeout       = 1;
        timeout.read_timeout_constant    = 0;
        timeout.read_timeout_multiplier  = 0;
        timeout.write_timeout_constant   = 0;
        timeout.write_timeout_multiplier = 0;
        auto* ser                        = new serial::Serial(port, baud_rate_, timeout);
        if (ser->isOpen()) {
            ports_[port] = ser;
            LOG_INFO("[SerialManager] Opened: " + port);
            if (!description.empty()) { LOG_INFO(description); }
            return true;
        }
        delete ser;
    } catch (const std::exception& e) {
        LOG_ERROR("[SerialManager] Failed to open " + port + ": " + e.what());
    }
    return false;
}

// Close ports that have disappeared, open ports that have appeared
void serial_manager_t::clean_ports() {
    const std::scoped_lock<std::mutex>  lock{mutex_};
    const std::vector<serial::PortInfo> available = serial::list_ports();
    for (const auto& info : available) {
        if (!ports_.contains(info.port)) { open_port(info.port, info.description); }
    }
    std::vector<std::string> to_remove;
    for (auto& [port, ser] : ports_) {
        bool found = false;
        for (const auto& info : available) {
            if (info.port == port) {
                found = true;
                break;
            }
        }
        if (!found) { to_remove.push_back(port); }
    }
    for (const auto& port : to_remove) { close_port(port); }
}
// Send Data to all selected Serial ports
void serial_manager_t::send_data(const std::string& msg) {
    const std::scoped_lock<std::mutex> lock{mutex_};
    std::vector<std::string>           failed;
    for (const auto& port : chosen_ports_) {
        auto it = ports_.find(port);
        if (it == ports_.end() || it->second == nullptr) { continue; }
        try {
            it->second->write(msg);
        } catch (const std::exception& e) {
            LOG_ERROR("[SerialManager] Write failed on " + port + ": " + e.what());
            failed.push_back(port);
        }
    }
    for (const auto& port : failed) { close_port(port); }
}

void serial_manager_t::receive_data() {
    const std::scoped_lock<std::mutex> lock{mutex_};
    std::vector<std::string>           failed;
    for (const auto& port : chosen_ports_) {
        auto it = ports_.find(port);
        if (it == ports_.end() || it->second == nullptr) { continue; }
        try {
            if (it->second->available() > 0) {
                auto input = it->second->read(it->second->available());
                input_stream_.push_back(input);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("[SerialManager] Read failed on " + port + ": " + e.what());
            failed.push_back(port);
        }
    }
    for (const auto& port : failed) { close_port(port); }
}
// Close selected port
void serial_manager_t::close_port(const std::string& port) {
    auto it = ports_.find(port);
    if (it == ports_.end()) { return; }
    it->second->close();
    delete it->second;
    ports_.erase(it);
    if (chosen_ports_.contains(port)) { chosen_ports_.erase(port); }
    LOG_INFO("[SerialManager] Closed: " + port);
}
// Close all ports
void serial_manager_t::close_all() {
    for (auto& [port, ser] : ports_) {
        ser->close();
        LOG_INFO("[SerialManager] Closed: " + port);
        delete ser;
    }
    ports_.clear();
    chosen_ports_.clear();
}
// change baud rate (its in the name)
void serial_manager_t::change_baud_rate(uint32_t baud) {
    baud_rate_ =
        static_cast<int>(baud); // TODO(blake) why is m_BaudRate an int but this func takes a u32
    for (auto& [port, ser] : ports_) {
        try {
            ser->setBaudrate(baud_rate_);
        } catch (const std::exception& e) {
            LOG_ERROR("[SerialManager] Baud rate change failed on " + port + ": " + e.what());
        }
    }
}

bool serial_manager_t::is_running() const { return worker_.joinable(); }

void serial_manager_t::stop() {
    if (!worker_.joinable()) { return; }
    keep_running = false;
    std::thread cleanup([worker = std::move(worker_), this]() mutable {
        if (worker.joinable()) { worker.join(); }
        close_all();
    });
    cleanup.detach();
}

bool serial_manager_t::is_port_selected(const std::string& port) {
    return chosen_ports_.contains(port);
}

void serial_manager_t::add_port(const std::string& port) { chosen_ports_.insert(port); }

void serial_manager_t::remove_port(const std::string& port) { chosen_ports_.erase(port); }

} // namespace mbr
