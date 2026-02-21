#pragma once
#include "core/Device.h"

class StatusBar
{
public:
    struct Status {
        uint8_t battery    = 0;      // 0-100
        bool    wifiOn     = false;
        bool    bluetoothOn = false;

        Status(uint8_t bat = 0, bool wifi = false, bool bt = false)
            : battery(bat), wifiOn(wifi), bluetoothOn(bt) {}
    };

    void render(const Status& status) {
        auto& lcd = Uni.Lcd;
        const uint16_t x = lcd.width() - WIDTH;

        // background
        lcd.fillRect(x, 0, WIDTH, lcd.height(), TFT_DARKGREY);

        // ─── Battery ──────────────────────────────────────
        _renderBattery(x, 4, status.battery);

        // ─── WiFi ─────────────────────────────────────────
        _renderIndicator(x, 48, "Wi", status.wifiOn);

        // ─── Bluetooth ────────────────────────────────────
        _renderIndicator(x, 80, "Bt", status.bluetoothOn);
    }

    static constexpr uint8_t WIDTH = 28;

private:
    void _renderBattery(uint16_t x, uint16_t y, uint8_t level) {
        auto& lcd = Uni.Lcd;

        // battery outline
        lcd.drawRect(x + 4, y, 20, 36, TFT_WHITE);
        lcd.drawFastHLine(x + 8, y - 2, 12, TFT_WHITE);   // battery tip

        // fill color based on level
        uint16_t color = TFT_GREEN;
        if (level < 20)      color = TFT_RED;
        else if (level < 50) color = TFT_ORANGE;

        // fill bar proportional to level
        uint8_t fillH = map(level, 0, 100, 0, 32);
        lcd.fillRect(x + 6, y + 2 + (32 - fillH), 16, fillH, color);

        // percentage text
        lcd.setTextSize(1);
        lcd.setTextColor(TFT_WHITE, TFT_DARKGREY);
        lcd.setCursor(x + 2, y + 40);
        lcd.printf("%3d%%", level);
    }

    void _renderIndicator(uint16_t x, uint16_t y, const char* label, bool active) {
        auto& lcd = Uni.Lcd;
        uint16_t bg = active ? TFT_BLUE : TFT_DARKGREY;
        uint16_t fg = active ? TFT_WHITE : TFT_LIGHTGREY;

        lcd.fillRoundRect(x + 2, y, WIDTH - 4, 24, 4, bg);
        lcd.setTextSize(1);
        lcd.setTextColor(fg, bg);
        lcd.setCursor(x + 5, y + 8);
        lcd.print(label);
    }
};