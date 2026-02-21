//
// Created by L Shaf on 2026-02-19.
//

#pragma once

#include "core/INavigation.h"
#include "M5HatMiniEncoderC.h"

M5HatMiniEncoderC encoder;

class EncoderNavigation : public INavigation
{
public:
  void begin() override
  {
    encoder.begin();
  }
  void update() override
  {
    const bool _isRotatedLeft = encoder.getEncoderValue() <= -2;
    const bool _isRotatedRight = encoder.getEncoderValue() >= 2;

    if (_isRotatedLeft || _isRotatedRight) encoder.resetCounter();

    if (!encoder.getButtonStatus())
      updateState(DIR_PRESS);
    else if (_isRotatedLeft)
      updateState(DIR_UP);
    else if (_isRotatedRight)
      updateState(DIR_DOWN);
    else
      updateState(DIR_NONE);
  }
};