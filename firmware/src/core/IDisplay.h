//
// Created by L Shaf on 2026-02-19.
//

#pragma once

#include <TFT_eSPI.h>

class IDisplay : public TFT_eSPI
{
public:
  // brightness percentage between 0 to 100
  virtual void setBrightness(uint8_t brightness) = 0;
  virtual void powerOff()
  {
    setBrightness(0);
  }
};
