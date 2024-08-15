#ifndef TFT
#define TFT

#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#include "bitmap/sanslab.h"
// #include "config.hpp"
extern int cursor_x_doc;

extern int BACKGROUND_HOME;
extern int BACKGROUND_DOCK;



void init_tft(Adafruit_ST7735 &tft);
void drawtext(Adafruit_ST7735 &tft, String text, uint16_t color, uint8_t x, uint8_t y, uint8_t textSize);

void displayStart(Adafruit_ST7735 &tft);
void setupDisplayIinit(Adafruit_ST7735 &tft);

void displayIcon(Adafruit_ST7735 &tft, bool wifi, bool eth, bool sd);
void displayTime(Adafruit_ST7735 &tft,String time, String day);
void displayData(Adafruit_ST7735 &tft, float volt, float current, float power, float energy);

#endif