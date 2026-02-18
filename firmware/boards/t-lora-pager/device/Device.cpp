//
// Created by L Shaf on 2026-02-18.
//

#include "device/Device.h"

CDevice Device;
void CDevice::begin()
{
  setBrightness(255);
  pinMode(TFT_BL, OUTPUT);
  setBrightness(100);
}

void CDevice::setBrightness(uint8_t brightness)
{
  if (brightness > 100) brightness = 100;
  const int bl = 5 + round(((255 - 5) * brightness / 100));
  analogWrite(TFT_BL, bl);
}

void CDevice::powerOff()
{
  //
}
