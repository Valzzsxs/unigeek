//
// Created by L Shaf on 2026-02-23.
//

#pragma once

#include "core/ScreenManager.h"
#include "ui/templates/BaseScreen.h"
#include <TFT_eSPI.h>

class MainMenuScreen : public BaseScreen
{
public:
  const char* title() override { return "Main Menu"; }

  void onInit() override;
  void onUpdate() override;
  void onRender() override;

  void onBack();
  void onItemSelected(uint8_t index);

private:
  typedef void (*DrawIconFunc)(TFT_eSPI& lcd, int16_t x, int16_t y, bool active);

  struct GridItem
  {
    const char* label;
    DrawIconFunc drawIcon;
  };

#ifdef APP_MENU_POWER_OFF
  static const uint8_t ITEM_COUNT = 8;
#else
  static const uint8_t ITEM_COUNT = 7;
#endif

  GridItem _items[ITEM_COUNT];

  uint8_t _selectedIndex = 0;

  bool _hasBackItem();
  uint8_t _effectiveCount();

  void _animateTransition(uint8_t fromIdx, uint8_t toIdx, int dir);
  void _renderFrame(uint8_t fromIdx, uint8_t toIdx, float progress, int dir);
  void _drawItem(TFT_eSprite& sprite, uint8_t idx, int16_t offsetY);

  static void _drawBackIcon(TFT_eSPI& lcd, int16_t x, int16_t y, bool active);
};
