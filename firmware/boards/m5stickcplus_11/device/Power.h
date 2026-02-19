//
// Created by L Shaf on 2026-02-19.
//

#pragma once
#include "AXP192.h"
#include "device/IPower.h"

class PowerImpl : public IPower
{
public:
  PowerImpl(AXP192* axp) : _axp(axp) {}
  void begin() override
  {
    _axp->begin();
  }
  int getBatteryPercentage() override
  {
    float b = _axp->GetBatVoltage();
    const int percent = static_cast<int>((b - 3.0) / 1.2) * 100;
    return (percent < 0) ? 1 : (percent >= 100) ? 100 : percent;

  }
  void powerOff() override
  {
    _axp->PowerOff();
  }
private:
  AXP192* _axp;
};
