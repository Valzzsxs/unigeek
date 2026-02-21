#pragma once

class IScreen
{
public:
  virtual ~IScreen() = default;

  virtual void init()   = 0;   // called once when screen becomes active
  virtual void update() = 0;   // called every loop
  virtual void render() = 0;   // called on demand when redraw needed
};