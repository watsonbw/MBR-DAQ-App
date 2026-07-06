#include "FS.h"

class SDCard {
  public:
    explicit SDCard();
    bool        OpenSD(const char* name);
    void        WriteSD(const char* msg);
    bool        CloseSD();
    bool        InitSD() const;
    bool        IsOpen  = false;
    bool        IsWrite = false;
    const char* Name;

  private:
    File      m_LogFile;
    const int m_Chipselect = 5;
    int64_t      m_LastFlush  = 0;
};
