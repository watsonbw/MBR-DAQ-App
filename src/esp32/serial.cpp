#include "esp32/serial.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <chrono>
#include <thread>
#include "serial/serial.h"
#include "core/log.hpp"

void Serial::Run(){
    /*
    while (true){
        Scan();
        WriteAll(m_Context->Backend->Data.GetCurrentLine());
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
        */
}


bool Serial::OpenPort(const std::string& port, const std::string& description) {
        try {
            auto* ser = new serial::Serial(port, m_baud_rate, serial::Timeout::simpleTimeout(m_timeout_ms));
            if (ser->isOpen()) {
                m_ports[port] = ser;
                LOG_INFO("[SerialManager] Opened: " + port);
                if (!description.empty()) std::cout << " (" << description << ")";
                std::cout << std::endl;
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
        if (m_ports.find(info.port) == m_ports.end()) {
            OpenPort(info.port, info.description);
        }
    }

    // Close ports that have disappeared
    std::vector<std::string> to_remove;
    for (auto& [port, ser] : m_ports) {
        bool found = false;
        for (const auto& info : available)
            if (info.port == port) { found = true; break; }
        if (!found) to_remove.push_back(port);
    }
    for (const auto& port : to_remove) ClosePort(port);
}

void Serial::WriteAll(const std::string& message) {
    std::vector<std::string> failed;
    for (auto& [port, ser] : m_ports) {
        try {
            ser->write(message);
        } catch (const std::exception& e) {
            LOG_ERROR("[SerialManager] Write failed on " + port + ": " + e.what());
            failed.push_back(port);
        }
    }
    for (const auto& port : failed) ClosePort(port);
}

void Serial::ClosePort(const std::string& port) {
        auto it = m_ports.find(port);
        if (it == m_ports.end()) return;
        it->second->close();
        delete it->second;
        m_ports.erase(it);
        LOG_INFO("[SerialManager] Closed: " + port);
    }

void Serial::CloseAll() {
        for (auto& [port, ser] : m_ports) {
            ser->close();
            delete ser;
        }
        m_ports.clear();
    }    