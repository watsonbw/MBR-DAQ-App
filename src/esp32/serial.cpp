#include <iostream>
#include <map>
#include <string>
#include <thread>
#include <vector>

#include "core/log.hpp"
#include "esp32/serial.hpp"
#include "serial/serial.h"

using namespace std::chrono_literals;

void Serial::Run() {
    while (true) {
        Scan();
        WriteAll(m_Context->Backend->Data.GetCurrentLine());
        std::this_thread::sleep_for(150ms);
        std::cout << "Hello!";
    }
}

bool Serial::OpenPort(const std::string& port, const std::string& description) {
    try {
        auto* ser =
            new serial::Serial(port, m_BaudRate, serial::Timeout::simpleTimeout(m_TimeoutMs));
        if (ser->isOpen()) {
            m_Ports[port] = ser;
            LOG_INFO("[SerialManager] Opened: " + port);
            if (!description.empty()) { std::cout << " (" << description << ")"; }
            std::cout << '\n';
            return true;
        }
        delete ser;
    } catch (const std::exception& e) {
        LOG_ERROR("[SerialManager] Failed to open " + port + ": " + e.what());
    }
    return false;
}

void Serial::Scan() {
    std::vector<serial::PortInfo> available = serial::list_ports();

    // Open newly discovered ports
    for (const auto& info : available) {
        if (!m_Ports.contains(info.port)) { OpenPort(info.port, info.description); }
    }

    // Close ports that have disappeared
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
    for (const auto& port : to_remove) {
        ClosePort(port);
    }
}

void Serial::WriteAll(const std::string& msg) {
    std::vector<std::string> failed;
    for (auto& [port, ser] : m_Ports) {
        try {
            ser->write(msg);
        } catch (const std::exception& e) {
            LOG_ERROR("[SerialManager] Write failed on " + port + ": " + e.what());
            failed.push_back(port);
        }
    }
    for (const auto& port : failed) {
        ClosePort(port);
    }
}

void Serial::ClosePort(const std::string& port) {
    auto it = m_Ports.find(port);
    if (it == m_Ports.end()) { return; }
    it->second->close();
    delete it->second;
    m_Ports.erase(it);
    LOG_INFO("[SerialManager] Closed: " + port);
}

void Serial::CloseAll() {
    for (auto& [port, ser] : m_Ports) {
        ser->close();
        delete ser;
    }
    m_Ports.clear();
}
