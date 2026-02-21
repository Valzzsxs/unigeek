// cpp
#include <Arduino.h>

#include "core/Device.h"
#include "core/ScreenManager.h"

#include "screens/MainMenuScreen.h"

static MainMenuScreen mainMenuScreen;

void setup() {
  Serial.begin(115200);
  Uni.begin();
  Screen.setScreen(&mainMenuScreen);
}

void loop() {
  Uni.update();
  Screen.update();
}
