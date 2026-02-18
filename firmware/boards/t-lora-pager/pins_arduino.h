//
// Created by L Shaf on 2026-02-15.
//

#pragma once

#define SPI_SS_PIN 21
#define SPI_MOSI_PIN 34
#define SPI_MISO_PIN 33
#define SPI_SCK_PIN 35
static const uint8_t SS = SPI_SS_PIN;
static const uint8_t MOSI = SPI_MOSI_PIN;
static const uint8_t SCK = SPI_MISO_PIN;
static const uint8_t MISO = SPI_SCK_PIN;

#define GROVE_SDA 3
#define GROVE_SCL 2
static const uint8_t SDA = GROVE_SDA;
static const uint8_t SCL = GROVE_SCL;

// TFT_eSPI display
#define ST7796_DRIVER 1
#define USER_SETUP_LOADED
#define USE_HSPI_PORT
#define SMOOTH_FONT 1
#define TFT_INVERSION_ON
#define TFT_WIDTH 222
#define TFT_HEIGHT 480
#define TFT_BL 42
#define TFT_MISO SPI_MISO_PIN
#define TFT_MOSI SPI_MOSI_PIN
#define TFT_SCLK SPI_SCK_PIN
#define TFT_CS 38
#define TFT_DC 37
#define SPI_FREQUENCY 80000000
#define SPI_READ_FREQUENCY 20000000

// Display Setup
#define ROTATION 3
#define MINBRIGHT (uint8_t)1