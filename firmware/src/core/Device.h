//
// Created by L Shaf on 2026-02-16.
//

#pragma once

class INavigation
{
public:
  enum Direction_t
  {
    DIR_UP,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT,
    DIR_PRESS
  };

  virtual ~INavigation() = default;
  virtual bool isPressed() const = 0;
  virtual Direction_t readDirection() const = 0;
};

class Device
{
public:
  static void begin();


private:

};