#pragma once

#include "ui/templates/BaseScreen.h"

class CharacterScreen : public BaseScreen
{
public:
  const char* title() override { return nullptr; }  // full-screen home — no header

  // Exception: skip BaseScreen chrome (header / status bar) for full-screen layout
  void update() override;
  void render() override;

  void onInit()   override;
  void onUpdate() override;
  void onRender() override;

private:
  unsigned long _lastRefreshMs = 0;

  // idle animation
  uint8_t       _animFrame   = 0;       // 0=normal, 1=blink
  unsigned long _lastAnimMs  = 0;

  // dialog bubble typing  (0=typing, 1=pausing, 2=deleting)
  uint8_t       _wordIdx     = 0;
  uint8_t       _wordPos     = 0;
  uint8_t       _wordState   = 0;
  unsigned long _lastCharMs  = 0;

  void _enterMainMenu();
};
