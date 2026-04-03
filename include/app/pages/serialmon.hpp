#pragma once

#include <memory>
#include <string>

#include "app/common/text.hpp"
#include "app/pages/page.hpp"
#include "esp32/serial.hpp"

enum class BaudRate : uint32_t {
    THREE = 300,
    TWELVE = 1200,
    TWENTYFOUR = 2400,
    FORTYEIGHT = 4800,
    NINETYSIX = 9600,
    ONENIGHTYTWO = 19200,
    THREEEIGHTYFOUR = 38400,
    FIVESEVENTYSIX = 57600,
    ONEONEFIFTYTWO = 115200,
};

class SerialPage : public Page {
  public:
    explicit SerialPage(const std::shared_ptr<AppContext>& ctx)
        : Page{ctx},  m_TextUtils{ctx} {}
    ~SerialPage() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update() override;

  private:
    void DrawTopLHS();
    void DrawTopRHS();

    void DrawBottom();


  private:
    TextUtils   m_TextUtils;
    bool        m_SendSerial;
    std::string m_SerialBuffer;
};