//
// M5StickC Plus 2 — ADC battery on GPIO 38, power hold on GPIO 4.
// No AXP192; uses direct GPIO + deep sleep for power off.
//

#pragma once
#include "core/IPower.h"
#include <driver/gpio.h>
#include <esp_sleep.h>

class PowerImpl : public IPower
{
public:
  void begin() override
  {
    // Keep device alive when USB disconnected
    pinMode(PWR_HOLD_PIN, OUTPUT);
    digitalWrite(PWR_HOLD_PIN, HIGH);

    // Battery ADC
    analogSetAttenuation(ADC_ATTENDB_MAX);
    pinMode(BAT_ADC_PIN, INPUT);
  }

  uint8_t getBatteryPercentage() override
  {
    // ADC reading with 2x voltage divider
    float voltage = analogReadMilliVolts(BAT_ADC_PIN) * 2.0 / 1000.0;
    int pct = (int)(((voltage - 3.0) / 1.2) * 100.0);
    if (pct < 1) pct = 1;
    if (pct > 100) pct = 100;
    return (uint8_t)pct;
  }

  void powerOff() override
  {
    digitalWrite(PWR_HOLD_PIN, LOW);
    esp_sleep_enable_ext0_wakeup((gpio_num_t)BTN_UP, LOW);
    esp_deep_sleep_start();
  }

  bool isCharging() override
  {
    return false;  // no charging detection on Plus 2
  }
};
