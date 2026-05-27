#include "FS.h"
#include "SD.h"
#include "SPI.h"

class SDCard {
  public:
    explicit SDCard();
    void OpenSD();
    void WriteSD(const char* msg);
    void CloseSD();

  private:
    File logFile;
    bool isOpen = 0;
    const int chipselect = 5;
    long lastFlush;
};
