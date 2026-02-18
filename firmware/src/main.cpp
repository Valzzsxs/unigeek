// cpp
#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include "core/Device.h"

auto tft = TFT_eSPI();

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting...");

  Device::begin();

  tft.init();
  tft.invertDisplay(true);   // ST7789 often needs this
  Serial.println("TFT init done");

  tft.setRotation(0);
  tft.fillScreen(TFT_RED);   // Use RED — easiest to see if anything works
  Serial.println("Fill done");
}

void loop() {
  Serial.println("Looper");
  delay(1000);
}
