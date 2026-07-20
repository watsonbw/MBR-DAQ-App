#include "esp32/serial.hpp"

#include <algorithm>
#include <exception>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <serial/serial.h>
#include <stdx/assert.hh>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "core/log.hpp"

using namespace std::chrono_literals;

namespace mbr {

[[nodiscard]] u32 baud_rate_value(baud_rate_t baud) noexcept {
    switch (baud) {
    case baud_rate_t::THREE:           return 300;
    case baud_rate_t::TWELVE:          return 1'200;
    case baud_rate_t::TWENTYFOUR:      return 2'400;
    case baud_rate_t::FORTYEIGHT:      return 4'800;
    case baud_rate_t::NINETYSIX:       return 9'600;
    case baud_rate_t::ONENIGHTYTWO:    return 19'200;
    case baud_rate_t::THREEEIGHTYFOUR: return 38'400;
    case baud_rate_t::FIVESEVENTYSIX:  return 57'600;
    case baud_rate_t::ONEONEFIFTYTWO:  return 115'200;
    default:                           UNREACHABLE("Unknown baud rate");
    }
}

[[nodiscard]] const char* baud_rate_string(baud_rate_t baud) noexcept {
    switch (baud) {
    case baud_rate_t::THREE:           return "300";
    case baud_rate_t::TWELVE:          return "1200";
    case baud_rate_t::TWENTYFOUR:      return "2400";
    case baud_rate_t::FORTYEIGHT:      return "4800";
    case baud_rate_t::NINETYSIX:       return "9600";
    case baud_rate_t::ONENIGHTYTWO:    return "19200";
    case baud_rate_t::THREEEIGHTYFOUR: return "38400";
    case baud_rate_t::FIVESEVENTYSIX:  return "57600";
    case baud_rate_t::ONEONEFIFTYTWO:  return "115200";
    default:                           UNREACHABLE("Unknown baud rate");
    }
}

serial_manager_t::~serial_manager_t() { stop(); }

// Initializes and maintains Serial behavior
void serial_manager_t::start() {
    if (worker_.joinable()) { return; }
    keep_running_ = true;
    worker_       = std::thread([this]() {
        i32       scan_counter     = 0;
        const i32 scan_counter_max = 200; // 200 * 5ms = 1 second
        while (keep_running_) {
            if (scan_counter++ >= scan_counter_max) {
                scan_counter = 0;
                this->clean_ports();
            }
            if (is_serial_write_) { this->receive_data(); }
            std::this_thread::sleep_for(5ms);
        }
    });
}
// Opens a port
bool serial_manager_t::open_port(const std::string& port, const std::string& description) {
    const std::scoped_lock lock{mutex_};
    try {
        serial::Timeout timeout;
        timeout.inter_byte_timeout       = 1;
        timeout.read_timeout_constant    = 0;
        timeout.read_timeout_multiplier  = 0;
        timeout.write_timeout_constant   = 0;
        timeout.write_timeout_multiplier = 0;
        auto ser = std::make_unique<serial::Serial>(port, baud_rate_value(baud_rate_), timeout);
        if (ser->isOpen()) {
            ports_.emplace(port, std::move(ser));
            log_info(log_, "[SerialManager] Opened: {}", port);
            if (!description.empty()) { log_info(log_, description); }
            return true;
        }
    } catch (const std::exception& e) {
        log_error(log_, "[SerialManager] Failed to open {}: {}", port, e.what());
    }
    return false;
}

// Close ports that have disappeared, open ports that have appeared
void serial_manager_t::clean_ports() {
    const std::vector<serial::PortInfo> available = serial::list_ports();
    const std::scoped_lock              lock{mutex_};
    for (const auto& info : available) {
        if (!ports_.contains(info.port)) { open_port(info.port, info.description); }
    }
    std::vector<std::string> to_remove;
    for (auto& [port, ser] : ports_) {
        if (!std::ranges::contains(available, port, &serial::PortInfo::port)) {
            to_remove.emplace_back(port);
        }
    }
    for (const auto& port : to_remove) { close_port(port); }
}

// A deep copy of all of the stored port names
std::vector<std::string> serial_manager_t::get_all_ports() const {
    const std::scoped_lock   lock{mutex_};
    std::vector<std::string> out;
    out.reserve(ports_.size());
    for (const auto& [port, _] : ports_) { out.emplace_back(port); }
    return out;
}

// TODO(blake): Is a copy really what we want here?
serial_manager_t::chosen_port_set_t serial_manager_t::get_chosen_ports() const {
    const std::scoped_lock lock{mutex_};
    return chosen_ports_;
}

// Send Data to all selected Serial ports
void serial_manager_t::send_data(const std::string& msg) {
    const std::scoped_lock   lock{mutex_};
    std::vector<std::string> failed;
    for (const auto& port : chosen_ports_) {
        auto it = ports_.find(port);
        if (it == ports_.end() || !it->second) { continue; }
        try {
            it->second->write(msg);
        } catch (const std::exception& e) {
            log_error(log_, "[SerialManager] Write failed on {}: {}", port, e.what());
            failed.push_back(port);
        }
    }
    for (const auto& port : failed) { close_port(port); }
}

void serial_manager_t::receive_data() {
    PROFILE_FUNCTION();
    const std::scoped_lock   lock{mutex_};
    std::vector<std::string> failed;
    for (const auto& port : chosen_ports_) {
        auto it = ports_.find(port);
        if (it == ports_.end() || !it->second) { continue; }
        try {
            if (it->second->available() > 0) {
                auto input = it->second->read(it->second->available());
                input_stream_.push_back(input);
                if (input_stream_.size() > 1'000) {
                    input_stream_.erase(input_stream_.begin(),
                                        input_stream_.begin() +
                                            static_cast<idiff>(input_stream_.size() - 1'000));
                }
            }
        } catch (const std::exception& e) {
            log_error(log_, "[SerialManager] Read failed on {}: {}", port, e.what());
            failed.push_back(port);
        }
    }
    for (const auto& port : failed) { close_port(port); }
}
// Close selected port
void serial_manager_t::close_port(const std::string& port) {
    const std::scoped_lock lock{mutex_};
    auto                   it = ports_.find(port);
    if (it == ports_.end()) { return; }
    it->second->close();
    ports_.erase(it);
    if (chosen_ports_.contains(port)) { chosen_ports_.erase(port); }
    log_info(log_, "[SerialManager] Closed: {}", port);
}
// Close all ports
void serial_manager_t::close_all() {
    const std::scoped_lock lock{mutex_};
    for (auto& [port, ser] : ports_) {
        try {
            ser->close();
        } catch (const std::exception& e) {
            log_info(log_, "[SerialManager] Failed to close port {}: ", e.what());
            continue;
        }
        log_info(log_, "[SerialManager] Closed: {}", port);
    }
    ports_.clear();
    chosen_ports_.clear();
}

void serial_manager_t::change_baud_rate(baud_rate_t baud) {
    const std::scoped_lock lock{mutex_};
    baud_rate_ = baud;
    for (auto& [port, ser] : ports_) {
        try {
            ser->setBaudrate(baud_rate_value(baud_rate_));
        } catch (const std::exception& e) {
            log_error(log_, "[SerialManager] Baud rate change failed on {}: {}", port, e.what());
        }
    }
}

bool serial_manager_t::is_running() const { return worker_.joinable(); }

void serial_manager_t::stop() {
    if (!worker_.joinable()) { return; }
    keep_running_ = false;
    worker_.join();
    close_all();
}

bool serial_manager_t::is_port_selected(const std::string& port) {
    const std::scoped_lock lock{mutex_};
    return chosen_ports_.contains(port);
}

void serial_manager_t::add_port(const std::string& port) {
    const std::scoped_lock lock{mutex_};
    chosen_ports_.insert(port);
}

void serial_manager_t::remove_port(const std::string& port) {
    const std::scoped_lock lock{mutex_};
    chosen_ports_.erase(port);
}

} // namespace mbr
