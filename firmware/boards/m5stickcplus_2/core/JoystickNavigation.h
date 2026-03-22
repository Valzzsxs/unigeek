//
// M5StickC Plus 2 — Joystick navigation with M5Hat Mini JoyC.
// Joystick XY maps to DIR_UP/DOWN/LEFT/RIGHT, joystick button = DIR_PRESS.
// BTN_A short press = DIR_BACK, BTN_UP = DIR_UP (fallback), BTN_B = DIR_DOWN (fallback).
// I2C address 0x54, connected via Grove (SDA=32, SCL=33).
// If JoyC is not detected on begin(), falls back to button-only navigation.
//

#pragma once

#include "core/INavigation.h"
// Avoid macro collision with M5HatMiniEncoderC (both define BUTTON_REG / RGB_LED_REG)
#undef BUTTON_REG
#undef RGB_LED_REG
#include "M5HatMiniJoyC.h"

M5HatMiniJoyC joyc;

class JoystickNavigation : public INavigation
{
public:
  void begin() override
  {
    // Probe I2C for JoyC before initializing
    Wire.begin(GROVE_SDA, GROVE_SCL, 200000L);
    Wire.beginTransmission(MiniJoyC_ADDR);
    _detected = (Wire.endTransmission() == 0);

    if (_detected) {
      joyc.begin(&Wire, MiniJoyC_ADDR, GROVE_SDA, GROVE_SCL, 200000L);
    }
  }

  void update() override
  {
    // BTN_A short press (< 3 s) = back; 3 s hold is handled in Device::boardHook()
    bool btnA = (digitalRead(BTN_A) == LOW);
    if (btnA && !_btnAWasLow) {
      _btnAStart  = millis();
      _btnAWasLow = true;
    } else if (!btnA && _btnAWasLow) {
      if (millis() - _btnAStart < 3000) _emitBack = true;
      _btnAWasLow = false;
    }
    if (_emitBack) { _emitBack = false; updateState(DIR_BACK); return; }

    // Fallback to buttons if JoyC not detected
    if (!_detected) {
      const bool btnUp = (digitalRead(BTN_UP) == LOW);
      const bool btnDn = (digitalRead(BTN_B)  == LOW);

      if (btnUp) updateState(DIR_UP);
      else if (btnDn) updateState(DIR_DOWN);
      else updateState(DIR_NONE);
      return;
    }

    // Joystick button = press
    bool joyBtn = !joyc.getButtonStatus();
    if (joyBtn && !_joyBtnWasDown) {
      _joyBtnWasDown = true;
    } else if (!joyBtn && _joyBtnWasDown) {
      _emitPress     = true;
      _joyBtnWasDown = false;
    }
    if (_emitPress) { _emitPress = false; updateState(DIR_PRESS); return; }

    // Joystick axis — 8-bit position: 0=min, 128=center, 255=max
    uint16_t x = joyc.getPOSValue(POS_X, _8bit);
    uint16_t y = joyc.getPOSValue(POS_Y, _8bit);

    // Deadzone: center ±_dz
    bool joyUp    = (y < 128 - _dz);
    bool joyDown  = (y > 128 + _dz);
    bool joyLeft  = (x < 128 - _dz);
    bool joyRight = (x > 128 + _dz);

    // Emit direction on initial tilt, then repeat after hold delay
    Direction dir = DIR_NONE;
    if      (joyUp)    dir = DIR_UP;
    else if (joyDown)  dir = DIR_DOWN;
    else if (joyLeft)  dir = DIR_LEFT;
    else if (joyRight) dir = DIR_RIGHT;

    if (dir != DIR_NONE) {
      unsigned long now = millis();
      if (!_joyHeld) {
        _joyHeld     = true;
        _joyDir      = dir;
        _joyHeldTime = now;
        _joyRepeat   = false;
        updateState(dir);
        return;
      } else if (dir == _joyDir) {
        unsigned long elapsed = now - _joyHeldTime;
        if (!_joyRepeat && elapsed >= _repeatDelay) {
          _joyRepeat   = true;
          _joyHeldTime = now;
          updateState(dir);
          return;
        } else if (_joyRepeat && elapsed >= _repeatInterval) {
          _joyHeldTime = now;
          updateState(dir);
          return;
        }
      } else {
        _joyDir      = dir;
        _joyHeldTime = now;
        _joyRepeat   = false;
        updateState(dir);
        return;
      }
    } else {
      _joyHeld = false;
    }

    updateState(DIR_NONE);
  }

private:
  static constexpr uint8_t       _dz             = 50;
  static constexpr unsigned long  _repeatDelay    = 400;
  static constexpr unsigned long  _repeatInterval = 150;

  bool          _detected      = false;

  // BTN_A back detection
  unsigned long _btnAStart     = 0;
  bool          _btnAWasLow    = false;
  bool          _emitBack      = false;

  // Joystick button
  bool          _joyBtnWasDown = false;
  bool          _emitPress     = false;

  // Joystick axis repeat
  bool          _joyHeld       = false;
  Direction     _joyDir        = DIR_NONE;
  unsigned long _joyHeldTime   = 0;
  bool          _joyRepeat     = false;
};
