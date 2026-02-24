// src/screens/SettingsScreen.h
#pragma once
#include "ui/templates/ListScreen.h"

class MainMenuScreen : public ListScreen
{
  const char* title() override { return "Main Menu"; }

  // list items
  ListItem _items[10] = {
    {"Brightness", "Adjust display"},
    {"LoRa Freq", "915 MHz"},
    {"Navigation", "Built-in"},
    {"Brightness", "Adjust display"},
    {"LoRa Freq", "915 MHz"},
    {"Navigation", "Built-in"},
    {"Brightness", "Adjust display"},
    {"LoRa Freq", "915 MHz"},
    {"Navigation", "Built-in"},
    {"Power Off"}
  };

  void onInit() override
  {
    setItems(_items);
    ListScreen::onInit();
  }

  void onItemSelected(uint8_t index) override {
    if (index == 9)
    {
      Serial.println("Power off called");
      Uni.Power.powerOff();
    }
  }
};