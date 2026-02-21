#pragma once
#include "core/Device.h"
#include "./StatusBar.h"

class Header
{
public:
  // pass nullptr to hide header entirely
  void render(const char* title) {
    if (!title) return;

    auto& lcd = Uni.Lcd;
    lcd.fillRect(0, 0, lcd.width() - StatusBar::WIDTH, HEIGHT, TFT_NAVY);
    lcd.setTextColor(TFT_WHITE, TFT_NAVY);
    lcd.setTextSize(2);
    lcd.setCursor(8, 8);
    lcd.print(title);
  }

  static constexpr uint8_t HEIGHT = 28;
};