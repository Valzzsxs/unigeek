#pragma once

#include "core/INavigation.h"
#include "pins_arduino.h"
#include <Arduino.h>

class NavigationImpl : public INavigation
{
private:
  uint32_t _lastDownPressTime = 0;
  bool _downBtnWasPressed = false;
  bool _waitingForDownDoubleClick = false;
  bool _syntheticDownTriggered = false;
  Direction _heldDownDirection = DIR_NONE;

  uint32_t _lastUpPressTime = 0;
  bool _upBtnWasPressed = false;
  bool _waitingForUpDoubleClick = false;
  bool _syntheticUpTriggered = false;
  Direction _heldUpDirection = DIR_NONE;

  // Timeout for double click in ms
  const uint32_t DOUBLE_CLICK_TIMEOUT = 250;

public:
  void begin() override {
    pinMode(UP_BTN, INPUT_PULLUP);
    pinMode(DW_BTN, INPUT_PULLUP);
  }

  void update() override
  {
    bool btnUp = (digitalRead(UP_BTN) == BTN_ACT);
    bool btnDown = (digitalRead(DW_BTN) == BTN_ACT);
    uint32_t now = millis();

    // Handle synthetic click generation when button is already released
    // We send the synthetic direction for one cycle, and then on the next cycle we send DIR_NONE to finish the click.
    if (_syntheticDownTriggered) {
      _syntheticDownTriggered = false;
      updateState(DIR_NONE);
      return;
    }

    if (_syntheticUpTriggered) {
      _syntheticUpTriggered = false;
      updateState(DIR_NONE);
      return;
    }

    // Process UP button
    if (btnUp && !_upBtnWasPressed) {
      _upBtnWasPressed = true;
      if (_waitingForUpDoubleClick && (now - _lastUpPressTime) <= DOUBLE_CLICK_TIMEOUT) {
        _waitingForUpDoubleClick = false;
        _heldUpDirection = DIR_BACK;
        updateState(DIR_BACK);
      } else {
        _waitingForUpDoubleClick = true;
        _lastUpPressTime = now;
        _heldUpDirection = DIR_NONE;
        updateState(DIR_NONE);
      }
      return;
    } else if (btnUp) {
      if (_waitingForUpDoubleClick) {
          if ((now - _lastUpPressTime) > DOUBLE_CLICK_TIMEOUT) {
              _waitingForUpDoubleClick = false;
              _heldUpDirection = DIR_UP;
              updateState(DIR_UP);
          } else {
              updateState(DIR_NONE);
          }
      } else {
          updateState(_heldUpDirection);
      }
      return;
    }

    // Process DOWN button
    if (btnDown && !_downBtnWasPressed) {
      _downBtnWasPressed = true;
      if (_waitingForDownDoubleClick && (now - _lastDownPressTime) <= DOUBLE_CLICK_TIMEOUT) {
        _waitingForDownDoubleClick = false;
        _heldDownDirection = DIR_PRESS;
        updateState(DIR_PRESS);
      } else {
        _waitingForDownDoubleClick = true;
        _lastDownPressTime = now;
        _heldDownDirection = DIR_NONE;
        updateState(DIR_NONE);
      }
      return;
    } else if (btnDown) {
      if (_waitingForDownDoubleClick) {
          if ((now - _lastDownPressTime) > DOUBLE_CLICK_TIMEOUT) {
              _waitingForDownDoubleClick = false;
              _heldDownDirection = DIR_DOWN;
              updateState(DIR_DOWN);
          } else {
              updateState(DIR_NONE);
          }
      } else {
          updateState(_heldDownDirection);
      }
      return;
    }

    // No buttons currently pressed
    _downBtnWasPressed = false;
    _upBtnWasPressed = false;
    _heldDownDirection = DIR_NONE;
    _heldUpDirection = DIR_NONE;

    if (_waitingForDownDoubleClick) {
      if ((now - _lastDownPressTime) > DOUBLE_CLICK_TIMEOUT) {
        _waitingForDownDoubleClick = false;
        _syntheticDownTriggered = true;
        updateState(DIR_DOWN);
        return;
      }
    }

    if (_waitingForUpDoubleClick) {
      if ((now - _lastUpPressTime) > DOUBLE_CLICK_TIMEOUT) {
        _waitingForUpDoubleClick = false;
        _syntheticUpTriggered = true;
        updateState(DIR_UP);
        return;
      }
    }

    updateState(DIR_NONE);
  }
};
