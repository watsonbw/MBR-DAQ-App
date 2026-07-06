#include "board_wifi.hpp"
#include "sd.hpp"
#include "rpm.hpp"

static constexpr int MAX_FILES    = 16;
static constexpr int MAX_NAME_LEN = 32;

class BoardBackend {
  public:
    explicit BoardBackend(const char* ssid, const char* password);
    ~BoardBackend() = default;

    void     Initialize();
    void     Run();
    uint64_t GetRealTime() const;
    void     CleanupClients() { m_Wifi.CleanupClients(); }

  public:
    // all data to be sent, this will likely get redone with canbus implementation
    volatile double WheelRPM  = 0;
    volatile double EngineRPM = 0;

  private:
    void                     SendData(const char* msg);
    void                     ReceiveData();
    const char*              m_SSID;
    const char*              m_Password;
    char                     m_Msg[200];
    int64_t                  m_BaseTimeMicros{0};
    int64_t                  m_LocalSyncMicros{0};
    volatile bool            m_IsTimeSynced{false};
    volatile bool            m_WifiOn{true};
    volatile bool            m_LoRaOn{false};
    static int64_t           m_LastSend;
    static int64_t           m_LastCleanup;
    BoardWifi                m_Wifi;
    SDCard                   m_Sd;
    RPMCollector             m_WheelRC;
    RPMCollector             m_EngineRC;
    char                     m_FileIndex[MAX_FILES][MAX_NAME_LEN]{};
    int                      m_FileCount = 0;
    std::vector<std::string> m_FileNames;
};
