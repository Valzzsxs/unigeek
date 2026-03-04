#pragma once

#include "core/Device.h"
#include "core/ConfigManager.h"
#include <TFT_eSPI.h>

class ShowProgressAction
{
public:
  // Show a progress overlay with a message and 0-100 percent fill bar.
  // Always returns immediately (non-blocking). Call repeatedly to update.
  static void show(const char* message, uint8_t percent) {
    ShowProgressAction action(message, percent);
    action._run();
  }

private:
  static constexpr int PAD   = 6;
  static constexpr int BAR_H = 10;
  static constexpr int MIN_W = 120;

  const char* _message;
  uint8_t     _percent;
  TFT_eSprite _spr;

  ShowProgressAction(const char* message, uint8_t percent)
    : _message(message), _percent(percent), _spr(&Uni.Lcd)
  {}

  void _run() {
    auto& lcd = Uni.Lcd;
    lcd.setTextSize(1);

    int textW = lcd.textWidth(_message);
    int w = max(textW + PAD * 4, MIN_W);
    int h = PAD + 12 + PAD + BAR_H + PAD;  // top + text + gap + bar + bottom
    int x = (lcd.width()  - w) / 2;
    int y = (lcd.height() - h) / 2;

    uint16_t barColor = Config.getThemeColor();

    _spr.createSprite(w, h);
    _spr.fillSprite(TFT_BLACK);
    _spr.drawRoundRect(0, 0, w, h, 4, TFT_WHITE);

    // Message text centered
    _spr.setTextColor(TFT_WHITE, TFT_BLACK);
    _spr.setTextDatum(MC_DATUM);
    _spr.setTextSize(1);
    _spr.drawString(_message, w / 2, PAD + 6);

    // Progress bar outline + fill
    int barX = PAD * 2;
    int barY = PAD + 12 + PAD;
    int barW = w - PAD * 4;
    _spr.drawRect(barX, barY, barW, BAR_H, barColor);
    int fillW = (int)(barW * _percent / 100.0f);
    if (fillW > 0)
      _spr.fillRect(barX + 1, barY + 1, fillW, BAR_H - 2, barColor);

    _spr.pushSprite(x, y);
    _spr.deleteSprite();
  }
};
