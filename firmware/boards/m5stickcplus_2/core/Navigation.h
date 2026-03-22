//
// M5StickC Plus 2 — 3-button navigation (all GPIO, no AXP).
// BTN_UP (GPIO 35) = UP, BTN_A (GPIO 37) = SELECT, BTN_B (GPIO 39) = DOWN
//

#pragma once

#include "core/INavigation.h"

class NavigationImpl : public INavigation
{
public:
  void begin() override {}

  void update() override
  {
    const bool btnUp  = (digitalRead(BTN_UP) == LOW);
    const bool btnSel = (digitalRead(BTN_A)  == LOW);
    const bool btnDn  = (digitalRead(BTN_B)  == LOW);

    if (btnSel) updateState(DIR_PRESS);
    else if (btnUp) updateState(DIR_UP);
    else if (btnDn) updateState(DIR_DOWN);
    else updateState(DIR_NONE);
  }
};
