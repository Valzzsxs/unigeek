//
// Created by L Shaf on 2026-02-18.
//
#include "device/Navigation.h"
#include <AXP192.h>

void Navigation::update()
{
  const bool btnUp = axpInstance.GetBtnPress();
  const bool btnDown = (digitalRead(PIN_BTN_DOWN) == LOW);
  const bool btnSel = (digitalRead(PIN_BTN_SEL) == LOW);

  if (btnSel) updateState(DIR_PRESS);
  else if (btnUp) updateState(DIR_UP);
  else if (btnDown) updateState(DIR_DOWN);
  else updateState(DIR_NONE);
}
