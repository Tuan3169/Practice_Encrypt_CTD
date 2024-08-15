#include "tft1_8.h"
#include "bitmap/icon.h"

int BACKGROUND_HOME = 0x07FD;
int BACKGROUND_DOCK = 0xF6AD;
int cursor_x_doc = 0;

void init_tft(Adafruit_ST7735 &tft) {
    tft.setSPISpeed(40000000);
    tft.initR(INITR_BLACKTAB);        // Initialize ST7735R screen
    tft.setRotation(1);
    Serial.println(F("init tft success"));
    
} 

void drawtext(Adafruit_ST7735 &tft ,String text, uint16_t color, uint8_t x, uint8_t y, uint8_t textSize) {
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(textSize);
  tft.setTextWrap(true);
  tft.print(text);
}

void displayStart(Adafruit_ST7735 &tft) {
    tft.fillScreen(ST77XX_WHITE);
    tft.drawRGBBitmap(40,0,sanslab,80,80);
    drawtext(tft,"CTD\n", ST77XX_ORANGE, 50, 80, 3);
    delay(2000);
}

void setupDisplayIinit(Adafruit_ST7735 &tft) {
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0,0);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.setTextWrap(true);
    tft.print("system initialization:\n");
}

void displayIcon(Adafruit_ST7735 &tft, bool wi, bool eth, bool sd) {
    tft.fillRect(0,0,65,18,BACKGROUND_DOCK);
    cursor_x_doc = 0;
    if(wi) {
        tft.drawBitmap(cursor_x_doc,2,wifi,16,16,ST7735_BLACK);
        cursor_x_doc += 18;
    }
    if(eth) {
        tft.drawBitmap(cursor_x_doc, 2, ethernet, 16, 16, ST7735_BLACK);
        cursor_x_doc += 18;
    }
    if(!wi & !eth) {
        tft.drawBitmap(cursor_x_doc, 2, no_internet, 16, 16, ST7735_BLACK);
        cursor_x_doc += 18;
    }
    if(sd) {
        tft.drawBitmap(cursor_x_doc, 2, micro_sd, 16, 16, ST7735_BLACK);
        cursor_x_doc += 18;
    }
}

void displayTime(Adafruit_ST7735 &tft,String time, String day) {
    tft.fillRect(65,0,104,18,BACKGROUND_DOCK);
    tft.setTextColor(ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(65,5);
    tft.print(time);
    tft.setCursor(100,5);
    tft.print(day);
}

void displayData(Adafruit_ST7735 &tft, float volt, float current, float power, float energy) {
    tft.setTextColor(0xF6AD);
    tft.setTextSize(2);
    tft.fillRect(0,34,160,20,BACKGROUND_HOME);
    tft.setCursor(12,34);
    tft.printf("V: %.1fV",volt);
    tft.fillRect(0,54,160,20,BACKGROUND_HOME);
    tft.setCursor(12,54);
    tft.printf("A: %.1fA", current);
    tft.fillRect(0,74,160,20,BACKGROUND_HOME);
    tft.setCursor(12,74);
    tft.printf("P: %.1fW", power);
    tft.fillRect(0,94,160,20,BACKGROUND_HOME);
    tft.setCursor(12,94);
    tft.printf("E: %.1fKWh", energy);
}