#pragma once

#include <atomic>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <map>

#include <serial/serial.h>
#include "app/context.hpp"
#include "esp32/data.hpp"

class Serial {
    public:
        Serial(const std::shared_ptr<AppContext>& ctx, int baud_rate = 115200, int timeout_ms = 500)
        : m_baud_rate(baud_rate), m_timeout_ms(timeout_ms), m_Context{ctx} {}
        ~Serial() { CloseAll(); };

        void WriteAll(const std::string& msg);
        void Run();
        void Scan();
        void ClosePort(const std::string& port);
        void CloseAll();
        bool OpenPort(const std::string& port, const std::string& description = "");
    
    private:
        int m_baud_rate;
        int m_timeout_ms;
        std::map<std::string, serial::Serial*> m_ports;
        std::shared_ptr<AppContext> m_Context;
};
