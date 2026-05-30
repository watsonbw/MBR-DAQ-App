#include "FS.h"
#include "SD.h"
#include "SPI.h"

class SDCard {
  public:
    explicit SDCard();
    bool OpenSD(const char* name);
    void WriteSD(const char* msg);
    bool CloseSD();
    bool isOpen = 0;
    bool isWrite = 0;
    const char* name;

  private:
    File logFile;
    const int chipselect = 5;
    long lastFlush = 0;
};
