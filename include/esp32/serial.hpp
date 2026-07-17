#pragma once

#include <map>
#include <mutex>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_set>

#include <serial/serial.h>

namespace mbr {

class SerialManager {
  public:
    explicit SerialManager(int baud_rate = 115'200, int timeout_ms = 0)
        : m_BaudRate(baud_rate), m_TimeoutMs(timeout_ms) {}
    ~SerialManager() {
        m_KeepRunning = false;
        if (m_Worker.joinable()) { m_Worker.join(); }
        CloseAll();
    };

    std::atomic<bool>                      m_KeepRunning{true};
    std::atomic<bool>                      m_SendData{false};
    std::atomic<bool>                      IsSerialWrite{false};
    std::map<std::string, serial::Serial*> ExportPorts() { return m_Ports; }
    std::unordered_set<std::string>        ReturnChosen() { return m_ChosenPorts; }
    std::vector<std::string>               ReturnDataStream() const { return m_InputStream; }
    void                                   SendData(const std::string& msg);
    void                                   ReceiveData();
    void                                   ReadAll();
    void                                   Start();
    void                                   AddPort(const std::string& port);
    void                                   RemovePort(const std::string& port);
    void                                   CleanPorts();
    void                                   ClosePort(const std::string& port);
    void                                   CloseAll();
    bool     OpenPort(const std::string& port, const std::string& description = "");
    bool     IsPortSelected(const std::string& port);
    void     ChangeBaudRate(uint32_t baud);
    void     Stop();
    bool     IsRunning() const;
    uint32_t GetBaudRate() { return m_BaudRate; }

  private:
    int                                    m_BaudRate;
    int                                    m_TimeoutMs;
    std::map<std::string, serial::Serial*> m_Ports;
    std::unordered_set<std::string>        m_ChosenPorts;
    std::thread                            m_Worker;
    std::vector<std::string>               m_InputStream;
    std::mutex                             m_Mutex;
};

} // namespace mbr
