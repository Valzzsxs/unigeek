// cpp
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "device/Device.h"

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting...");

  Uni.begin();

  Uni.Lcd.setRotation(3);
  Uni.Lcd.fillScreen(TFT_BLACK);
  Uni.Lcd.setTextDatum(TC_DATUM);
  Uni.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  Serial.println("Fill done");
}

void loop() {
  Uni.update();
  if (Uni.Nav.wasPressed())
  {
    switch (Uni.Nav.readDirection())
    {
      case INavigation::DIR_UP:
        Uni.Lcd.drawString(" UP ", Uni.Lcd.width() / 2, 2);
        break;
      case INavigation::DIR_DOWN:
        Uni.Lcd.drawString(" DW ", Uni.Lcd.width() / 2, 2);
        break;
      case INavigation::DIR_PRESS:
        Uni.Lcd.drawString(" SL ", Uni.Lcd.width() / 2, 2);
        break;
      default:
        Serial.println("NONE");
    }
  }
}
