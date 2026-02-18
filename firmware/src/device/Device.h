//
// Created by L Shaf on 2026-02-16.
//

#pragma once

#include <TFT_eSPI.h>
#include "Navigation.h"

class CDevice
{
public:
  void begin();
  void setBrightness(uint8_t brightness);
  void powerOff();
  void update();

  Navigation Nav;
  TFT_eSPI Lcd;
};

extern CDevice Device;