// cpp
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "device/Device.h"

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting...");

  Device.begin();

  Device.Lcd.setRotation(3);
  Device.Lcd.fillScreen(TFT_BLACK);
  Device.Lcd.setTextDatum(TC_DATUM);
  Device.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  Serial.println("Fill done");
}

void loop() {
  Device.update();
  if (Device.Nav.wasPressed())
  {
    switch (Device.Nav.readDirection())
    {
      case Navigation::DIR_UP:
        Device.Lcd.drawString(" UP ", Device.Lcd.width() / 2, 2);
        break;
      case Navigation::DIR_DOWN:
        Device.Lcd.drawString(" DW ", Device.Lcd.width() / 2, 2);
        break;
      case Navigation::DIR_PRESS:
        Device.Lcd.drawString(" SL ", Device.Lcd.width() / 2, 2);
        break;
      default:
        Serial.println("NONE");
    }
  }
}
