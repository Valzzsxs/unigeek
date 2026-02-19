//
// Created by L Shaf on 2026-02-19.
//

#pragma once

class IKeyboard
{
public:
  virtual ~IKeyboard() = default;
  virtual void begin() = 0;
};