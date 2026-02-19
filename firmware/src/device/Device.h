//
// Created by L Shaf on 2026-02-19.
//

#pragma once

#include "INavigation.h"
#include "IDisplay.h"
#include "IPower.h"
#include "IKeyboard.h"

class Device
{
public:
  static Device& getInstance() {
    static Device* instance = createInstance();  // ← pointer, no copy/default ctor needed
    return *instance;
  }

  void begin()
  {
    Lcd.begin();
    Lcd.setBrightness(100);
    Lcd.setRotation(0);
    Lcd.invertDisplay(true);

    Power.begin();
    Nav.begin();
  }

  void update()
  {
    Nav.update();
  }

  INavigation& Nav;
  IDisplay& Lcd;
  IPower& Power;
  IKeyboard*   Keyboard = nullptr;

  // Prevent copying
  Device(const Device&)            = delete;
  Device& operator=(const Device&) = delete;
private:
  // Private constructor — takes concrete implementations
  Device(IDisplay& lcd, IPower& power, INavigation& nav, IKeyboard* keyboard = nullptr)
      : Lcd(lcd), Power(power), Nav(nav), Keyboard(keyboard) {}

  // Returns a heap-allocated instance — defined in Device.cpp
  static Device* createInstance();
};


// Global access macro for convenience
#define Uni Device::getInstance()