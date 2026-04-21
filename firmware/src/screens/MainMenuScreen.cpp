//
// Created by L Shaf on 2026-02-23.
//

#include "MainMenuScreen.h"
#include "screens/wifi/WifiMenuScreen.h"
#include "screens/ble/BLEMenuScreen.h"
#include "screens/keyboard/KeyboardMenuScreen.h"
#include "screens/game/GameMenuScreen.h"
#include "screens/module/ModuleMenuScreen.h"
#include "screens/utility/UtilityMenuScreen.h"
#include "screens/setting/SettingScreen.h"
#include "screens/CharacterScreen.h"
#include "ui/components/Icon.h"

void MainMenuScreen::_drawBackIcon(TFT_eSPI& lcd, int16_t x, int16_t y, bool active) {
  uint16_t color = active ? TFT_CYAN : TFT_WHITE;
  lcd.drawLine(x + 14, y + 4, x + 8, y + 10, color);
  lcd.drawLine(x + 8, y + 10, x + 14, y + 16, color);
  lcd.drawLine(x + 15, y + 4, x + 9, y + 10, color);
  lcd.drawLine(x + 9, y + 10, x + 15, y + 16, color);
}

void MainMenuScreen::onInit() {
  _items[0] = {"Wifi", Icons::drawWifi};
  _items[1] = {"Bluetooth", Icons::drawBluetooth};
  _items[2] = {"Keyboard", Icons::drawKeyboard};
  _items[3] = {"Modules", Icons::drawModule};
  _items[4] = {"Utility", Icons::drawUtility};
  _items[5] = {"Games", Icons::drawGame};
  _items[6] = {"Settings", Icons::drawSetting};
#ifdef APP_MENU_POWER_OFF
  _items[7] = {"Power Off", Icons::drawPower};
#endif

  _selectedIndex = 0;
}

bool MainMenuScreen::_hasBackItem()
{
#ifdef DEVICE_HAS_KEYBOARD
  return false;
#else
#ifdef DEVICE_HAS_NAV_MODE_SWITCH
  if (Config.get(APP_CONFIG_NAV_MODE, APP_CONFIG_NAV_MODE_DEFAULT) == "encoder")
    return false;
#endif
  return true;
#endif
}

uint8_t MainMenuScreen::_effectiveCount()
{
  return ITEM_COUNT + (_hasBackItem() ? 1 : 0);
}

void MainMenuScreen::onUpdate() {
  if (Uni.Nav->wasPressed()) {
    auto dir = Uni.Nav->readDirection();

    if (dir == INavigation::DIR_BACK) {
      onBack();
      return;
    }

    uint8_t eff = _effectiveCount();
    if (eff == 0) return;

    if (dir == INavigation::DIR_UP || dir == INavigation::DIR_LEFT) {
      uint8_t nextIdx = (_selectedIndex == 0) ? eff - 1 : _selectedIndex - 1;
      _animateTransition(_selectedIndex, nextIdx, -1);
      if (Uni.Speaker) Uni.Speaker->beep();
    }
    else if (dir == INavigation::DIR_DOWN || dir == INavigation::DIR_RIGHT) {
      uint8_t nextIdx = (_selectedIndex >= eff - 1) ? 0 : _selectedIndex + 1;
      _animateTransition(_selectedIndex, nextIdx, 1);
      if (Uni.Speaker) Uni.Speaker->beep();
    }
    else if (dir == INavigation::DIR_PRESS) {
#ifndef DEVICE_HAS_KEYBOARD
      if (_hasBackItem() && _selectedIndex == ITEM_COUNT) { onBack(); return; }
#endif
      onItemSelected(_selectedIndex);
    }
  }
}

void MainMenuScreen::_animateTransition(uint8_t fromIdx, uint8_t toIdx, int dir) {
  const int frames = 12;
  for (int i = 1; i <= frames; i++) {
    // ease-out
    float t = (float)i / frames;
    float progress = 1.0f - (1.0f - t) * (1.0f - t);

    _renderFrame(fromIdx, toIdx, progress, dir);
    delay(10);
  }
  _selectedIndex = toIdx;
  onRender();
}

void MainMenuScreen::_renderFrame(uint8_t fromIdx, uint8_t toIdx, float progress, int dir) {
  auto& lcd = Uni.Lcd;
  TFT_eSprite sprite(&lcd);
  sprite.createSprite(bodyW(), bodyH());
  sprite.fillSprite(TFT_BLACK);

  int16_t maxOffset = bodyH();
  int16_t offset = (int16_t)(progress * maxOffset) * dir;

  if (progress < 1.0f) {
    _drawItem(sprite, fromIdx, -offset);
    _drawItem(sprite, toIdx, (dir > 0 ? maxOffset : -maxOffset) - offset);
  } else {
    _drawItem(sprite, toIdx, 0);
  }

  sprite.pushSprite(bodyX(), bodyY());
  sprite.deleteSprite();
}

void MainMenuScreen::_drawItem(TFT_eSprite& sprite, uint8_t idx, int16_t offsetY) {
  static const GridItem _backGridItem = {"Back", _drawBackIcon};

  const GridItem* item;
  if (_hasBackItem() && idx == ITEM_COUNT)
    item = &_backGridItem;
  else
    item = &_items[idx];

  // Draw scaled icon
  auto& lcd = Uni.Lcd;
  TFT_eSprite tmpIcon(&lcd);
  tmpIcon.createSprite(24, 24);
  tmpIcon.fillSprite(TFT_BLACK);
  item->drawIcon(tmpIcon, 0, 0, true);

  uint8_t scale = 3;
  int16_t iconX = (bodyW() - (24 * scale)) / 2;
  int16_t iconY = (bodyH() - (24 * scale)) / 2 - 10 + offsetY;

  for (int y = 0; y < 24; y++) {
    for (int x = 0; x < 24; x++) {
      uint16_t color = tmpIcon.readPixel(x, y);
      if (color != TFT_BLACK) {
        sprite.fillRect(iconX + x * scale, iconY + y * scale, scale, scale, color);
      }
    }
  }
  tmpIcon.deleteSprite();

  // Draw label
  sprite.setTextColor(TFT_WHITE, TFT_BLACK);
  sprite.setTextDatum(TC_DATUM);
  sprite.setTextSize(2);
  sprite.drawString(item->label, bodyW() / 2, iconY + (24 * scale) + 10, 1);
  sprite.setTextSize(1);
}

void MainMenuScreen::onRender() {
  uint8_t eff = _effectiveCount();
  if (eff == 0) return;
  _renderFrame(_selectedIndex, _selectedIndex, 1.0f, 0);
}

void MainMenuScreen::onBack() {
  Screen.setScreen(new CharacterScreen());
}

void MainMenuScreen::onItemSelected(uint8_t index) {
  switch (index) {
  case 0: Screen.setScreen(new WifiMenuScreen());      break;
  case 1: Screen.setScreen(new BLEMenuScreen());       break;
  case 2: Screen.setScreen(new KeyboardMenuScreen());  break;
  case 3: Screen.setScreen(new ModuleMenuScreen());    break;
  case 4: Screen.setScreen(new UtilityMenuScreen());   break;
  case 5: Screen.setScreen(new GameMenuScreen());      break;
  case 6: Screen.setScreen(new SettingScreen());       break;
#ifdef APP_MENU_POWER_OFF
  case 7: Uni.Power.powerOff(); break;
#endif
  }
}