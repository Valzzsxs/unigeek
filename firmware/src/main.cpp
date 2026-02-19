// cpp
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <vector>

#include "device/Device.h"
#include "device/EncoderNavigation.h"

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting...");

  Uni.begin();

  Uni.Lcd.setRotation(3);
  Uni.Lcd.fillScreen(TFT_BLACK);
  Uni.Lcd.setTextDatum(TC_DATUM);
  Uni.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  Uni.Lcd.setTextSize(3);
  Serial.println("Fill done");
}

void loop() {
  static int8_t counter = 0;
  std::vector<int> colors = {TFT_BLUE, TFT_WHITE, TFT_GREEN, TFT_YELLOW, TFT_RED};
  Uni.update();
  if (Uni.Nav->wasPressed())
  {
    const auto dir = Uni.Nav->readDirection();
#ifdef HAS_ENCODER_NAV
    if (dir == INavigation::DIR_PRESS && Uni.Nav->pressDuration() > 1000)
    {
      Uni.switchNavigation(new EncoderNavigation());
      Uni.Lcd.drawString(" Encoder ", Uni.Lcd.width() / 2, Uni.Lcd.height() / 2 - Uni.Lcd.fontHeight() / 2);
    }
#endif
    switch (dir)
    {
      case INavigation::DIR_UP:
        counter = (counter + 1) % 5;
        break;
      case INavigation::DIR_DOWN:
        counter = (counter - 1) % 5;
        break;
      case INavigation::DIR_PRESS:
        counter = 0;
        break;
      default:
        Serial.println("NONE");
    }

    if (counter < 0) counter += 5;
    Uni.Lcd.setTextColor(colors[counter], TFT_BLACK);
    Uni.Lcd.drawString(" HELLO WORLD ", Uni.Lcd.width() / 2, Uni.Lcd.height() / 2 - Uni.Lcd.fontHeight() / 2);
  }
}
