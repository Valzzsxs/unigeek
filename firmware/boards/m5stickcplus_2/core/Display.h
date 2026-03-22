//
// M5StickC Plus 2 — PWM backlight on GPIO 27 (no AXP192).
//

#pragma once

#include "core/IDisplay.h"

static constexpr uint8_t BL_MIN_DUTY  = 160;  // minimum visible brightness (out of 255)

class DisplayImpl : public IDisplay
{
public:
  void initBacklight()
  {
    ledcAttach(LCD_BL, 1000, 8);
    ledcWrite(LCD_BL, 255);
  }

  void setBrightness(uint8_t pct) override
  {
    if (pct > 100) pct = 100;
    if (pct == 0) {
      ledcWrite(LCD_BL, 0);
    } else {
      uint8_t duty = BL_MIN_DUTY + ((255 - BL_MIN_DUTY) * pct / 100);
      ledcWrite(LCD_BL, duty);
    }
  }
};
