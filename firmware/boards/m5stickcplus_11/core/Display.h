//
// Created by L Shaf on 2026-02-19.
//
#pragma once

#include <AXP192.h>
#include "core/IDisplay.h"

class DisplayImpl : public IDisplay
{
public:
  DisplayImpl(AXP192* axp) : _axp(axp) {}
  void setBrightness(uint8_t brightness) override
  {
    if (brightness > 100) brightness = 100;
    if (brightness < 2) brightness = 2;
    _axp->ScreenBreath(brightness);
  }
private:
  AXP192* _axp;
};