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
void SerialManager::Start() {
    if (m_Worker.joinable()) { return; }
    m_KeepRunning = true;
    m_Worker      = std::thread([this]() {
        while (m_KeepRunning) {
            this->CleanPorts();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (IsSerialWrite) { this->ReceiveData(); }
        }
    });
}
// Opens a port
bool SerialManager::OpenPort(const std::string& port, const std::string& description) {
    try {
        serial::Timeout timeout;
        timeout.inter_byte_timeout       = 1;
        timeout.read_timeout_constant    = 0;
        timeout.read_timeout_multiplier  = 0;
        timeout.write_timeout_constant   = 0;
        timeout.write_timeout_multiplier = 0;
        auto* ser                        = new serial::Serial(port, m_BaudRate, timeout);
        if (ser->isOpen()) {
            m_Ports[port] = ser;
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
void SerialManager::CleanPorts() {
    const std::scoped_lock<std::mutex>  lock{m_Mutex};
    const std::vector<serial::PortInfo> available = serial::list_ports();
    for (const auto& info : available) {
        if (!m_Ports.contains(info.port)) { OpenPort(info.port, info.description); }
    }
    std::vector<std::string> to_remove;
    for (auto& [port, ser] : m_Ports) {
        bool found = false;
        for (const auto& info : available) {
            if (info.port == port) {
                found = true;
                break;
            }
        }
        if (!found) { to_remove.push_back(port); }
    }
    for (const auto& port : to_remove) { ClosePort(port); }
}
// Send Data to all selected Serial ports
void SerialManager::SendData(const std::string& msg) {
    const std::scoped_lock<std::mutex> lock{m_Mutex};
    std::vector<std::string>           failed;
    for (const auto& port : m_ChosenPorts) {
        auto it = m_Ports.find(port);
        if (it == m_Ports.end() || it->second == nullptr) { continue; }
        try {
            it->second->write(msg);
        } catch (const std::exception& e) {
            LOG_ERROR("[SerialManager] Write failed on " + port + ": " + e.what());
            failed.push_back(port);
        }
    }
    for (const auto& port : failed) { ClosePort(port); }
}

void SerialManager::ReceiveData() {
    const std::scoped_lock<std::mutex> lock{m_Mutex};
    std::vector<std::string>           failed;
    for (const auto& port : m_ChosenPorts) {
        auto it = m_Ports.find(port);
        if (it == m_Ports.end() || it->second == nullptr) { continue; }
        try {
            if (it->second->available() > 0) {
                auto input = it->second->read(it->second->available());
                m_InputStream.push_back(input);
            }
        } catch (const std::exception& e) {
            LOG_ERROR("[SerialManager] Read failed on " + port + ": " + e.what());
            failed.push_back(port);
        }
    }
    for (const auto& port : failed) { ClosePort(port); }
}
// Close selected port
void SerialManager::ClosePort(const std::string& port) {
    auto it = m_Ports.find(port);
    if (it == m_Ports.end()) { return; }
    it->second->close();
    delete it->second;
    m_Ports.erase(it);
    if (m_ChosenPorts.contains(port)) { m_ChosenPorts.erase(port); }
    LOG_INFO("[SerialManager] Closed: " + port);
}
// Close all ports
void SerialManager::CloseAll() {
    for (auto& [port, ser] : m_Ports) {
        ser->close();
        LOG_INFO("[SerialManager] Closed: " + port);
        delete ser;
    }
    m_Ports.clear();
    m_ChosenPorts.clear();
}
// change baud rate (its in the name)
void SerialManager::ChangeBaudRate(uint32_t baud) {
    m_BaudRate =
        static_cast<int>(baud); // TODO(blake) why is m_BaudRate an int but this func takes a u32
    for (auto& [port, ser] : m_Ports) {
        try {
            ser->setBaudrate(m_BaudRate);
        } catch (const std::exception& e) {
            LOG_ERROR("[SerialManager] Baud rate change failed on " + port + ": " + e.what());
        }
    }
}

bool SerialManager::IsRunning() const { return m_Worker.joinable(); }

void SerialManager::Stop() {
    if (!m_Worker.joinable()) { return; }
    m_KeepRunning = false;
    std::thread cleanup([worker = std::move(m_Worker), this]() mutable {
        if (worker.joinable()) { worker.join(); }
        CloseAll();
    });
    cleanup.detach();
}

bool SerialManager::IsPortSelected(const std::string& port) { return m_ChosenPorts.contains(port); }

void SerialManager::AddPort(const std::string& port) { m_ChosenPorts.insert(port); }

void SerialManager::RemovePort(const std::string& port) { m_ChosenPorts.erase(port); }

} // namespace mbr
