//
// Created by L Shaf on 2026-02-16.
//

#include "device/Device.h"
#include <AXP192.h>

CDevice Device;
void CDevice::begin()
{
  axpInstance.begin();
  Lcd.begin();
  Lcd.setRotation(0);
  Lcd.invertDisplay(true);
  setBrightness(100);

  pinMode(PIN_BTN_SEL, INPUT);
  pinMode(PIN_BTN_DOWN, INPUT);
}

void CDevice::setBrightness(uint8_t brightness)
{
  if (brightness > 100) brightness = 100;
  if (brightness < 5) brightness = 5;
  axpInstance.ScreenBreath(brightness);
}

void CDevice::powerOff()
{
  axpInstance.PowerOff();
}

void CDevice::update()
{
  Nav.update();
}
