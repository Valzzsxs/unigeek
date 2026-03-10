#pragma once

#include "core/INavigation.h"
#include "pins_arduino.h"
#include <Arduino.h>

class NavigationImpl : public INavigation
{
private:
  bool _dualPressed = false;

public:
  void begin() override {
    pinMode(UP_BTN, INPUT_PULLUP);
    pinMode(DW_BTN, INPUT_PULLUP);
  }

  void update() override
  {
    bool btnUp = (digitalRead(UP_BTN) == BTN_ACT);
    bool btnDown = (digitalRead(DW_BTN) == BTN_ACT);

    if (btnUp && btnDown) {
      _dualPressed = true;
      updateState(DIR_PRESS);
    } else if (btnUp || btnDown) {
      if (_dualPressed) {
        updateState(DIR_PRESS);
      } else {
        updateState(btnUp ? DIR_UP : DIR_DOWN);
      }
    } else {
      _dualPressed = false;
      updateState(DIR_NONE);
    }
  }
};
