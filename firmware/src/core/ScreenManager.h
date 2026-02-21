#pragma once
#include "IScreen.h"

class ScreenManager
{
public:
  static ScreenManager& getInstance() {
    static ScreenManager instance;
    return instance;
  }

  void setScreen(IScreen* screen) {
    _current = screen;
    if (_current) _current->init();
  }

  void update() {
    if (_current) _current->update();
  }

  void render() {
    if (_current) _current->render();
  }

  IScreen* current() { return _current; }

  ScreenManager(const ScreenManager&)            = delete;
  ScreenManager& operator=(const ScreenManager&) = delete;

private:
  ScreenManager() = default;
  IScreen* _current = nullptr;
};

#define Screen ScreenManager::getInstance()