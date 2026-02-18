//
// Created by L Shaf on 2026-02-15.
//

#pragma once

#define GROVE_SDA 32
#define GROVE_SCL 33
static const uint8_t SDA = GROVE_SDA;
static const uint8_t SCL = GROVE_SCL;

//  Main SPI Bus
#define SPI_SS_PIN 5
#define SPI_MOSI_PIN 15
#define SPI_MISO_PIN 36
#define SPI_SCK_PIN 13
static const uint8_t SS    = SPI_SS_PIN;
static const uint8_t MOSI  = SPI_MOSI_PIN;
static const uint8_t MISO  = SPI_MISO_PIN;
static const uint8_t SCK   = SPI_SCK_PIN;

#define DISABLE_ALL_LIBRARY_WARNINGS 1
#define USER_SETUP_LOADED 1
#define ST7789_2_DRIVER 1
#define TFT_RGB_ORDER 1
#define TFT_WIDTH 135
#define TFT_HEIGHT 240
#define TFT_BACKLIGHT_ON 0
#define TFT_CS 5
#define TFT_DC 23
#define TFT_RST 18
#define TFT_MOSI 15
#define TFT_SCLK 13
#define TFT_BL -1
#define SMOOTH_FONT 1
#define SPI_FREQUENCY 20000000
#define SPI_READ_FREQUENCY 20000000
#define SPI_TOUCH_FREQUENCY 2500000
