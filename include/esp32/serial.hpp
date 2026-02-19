#pragma once

#include <map>
#include <string>

#include "app/context.hpp"
#include "esp32/data.hpp"
#include <serial/serial.h>

class Serial {
  public:
    explicit Serial(const std::shared_ptr<AppContext>& ctx,
                    int                                baud_rate  = 115200,
                    int                                timeout_ms = 500)
        : m_BaudRate(baud_rate), m_TimeoutMs(timeout_ms), m_Context{ctx} {}
    ~Serial() { CloseAll(); };

    void WriteAll(const std::string& msg);
    void Run();
    void Scan();
    void ClosePort(const std::string& port);
    void CloseAll();
    bool OpenPort(const std::string& port, const std::string& description = "");

  private:
    int                                    m_BaudRate;
    int                                    m_TimeoutMs;
    std::map<std::string, serial::Serial*> m_Ports;
    std::shared_ptr<AppContext>            m_Context;
};
