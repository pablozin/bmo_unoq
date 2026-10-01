//Guarda os pinos, comandos SPI do display e as funções de desenho geométrico

#pragma once
#include <SPI.h>
#include <math.h>

#define TFT_CS   10
#define TFT_RST  8
#define TFT_DC   9

#define ST7735_SWRESET 0x01
#define ST7735_SLPOUT  0x11
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_MADCTL  0x36
#define ST7735_COLMOD  0x3A

// Paleta de Cores
#define BMO_TEAL      0x5EB6
#define GADGET_BEIGE  0xEF58
#define CANDY_PINK    0xF973
#define CANDY_YELLOW  0xFFE0
#define DARK_TEAL     0x3CAE
#define BLACK         0x0000
#define WHITE         0xFFFF

inline void writeCommand(uint8_t cmd) {
  digitalWrite(TFT_DC, LOW); digitalWrite(TFT_CS, LOW);
  SPI.transfer(cmd); digitalWrite(TFT_CS, HIGH);
}

inline void writeData(uint8_t data) {
  digitalWrite(TFT_DC, HIGH); digitalWrite(TFT_CS, LOW);
  SPI.transfer(data); digitalWrite(TFT_CS, HIGH);
}

inline void setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
  writeCommand(ST7735_CASET); writeData(0x00); writeData(x0); writeData(0x00); writeData(x1);
  writeCommand(ST7735_RASET); writeData(0x00); writeData(y0); writeData(0x00); writeData(y1);
  writeCommand(ST7735_RAMWR);
}

void fillScreen(uint16_t color) {
  setAddrWindow(0, 0, 127, 159);
  digitalWrite(TFT_DC, HIGH); digitalWrite(TFT_CS, LOW);
  for (uint32_t i = 0; i < 128UL * 160; i++) {
    SPI.transfer(color >> 8); SPI.transfer(color & 0xFF);
  }
  digitalWrite(TFT_CS, HIGH);
}

void drawPixel(int x, int y, uint16_t color) {
  if (x < 0 || x >= 128 || y < 0 || y >= 160) return;
  setAddrWindow(x, y, x, y);
  digitalWrite(TFT_DC, HIGH); digitalWrite(TFT_CS, LOW);
  SPI.transfer(color >> 8); SPI.transfer(color & 0xFF);
  digitalWrite(TFT_CS, HIGH);
}

void fillRect(int x, int y, int w, int h, uint16_t color) {
  if (x >= 128 || y >= 160) return;
  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }
  if (x + w > 128) w = 128 - x;
  if (y + h > 160) h = 160 - y;
  if (w <= 0 || h <= 0) return;

  setAddrWindow(x, y, x + w - 1, y + h - 1);
  digitalWrite(TFT_DC, HIGH); digitalWrite(TFT_CS, LOW);
  for (int i = 0; i < w * h; i++) {
    SPI.transfer(color >> 8); SPI.transfer(color & 0xFF);
  }
  digitalWrite(TFT_CS, HIGH);
}

void fillCircle(int x0, int y0, int r, uint16_t color) {
  for (int y = -r; y <= r; y++) {
    int x = (int)sqrt(r * r - y * y);
    fillRect(x0 - x, y0 + y, x * 2 + 1, 1, color);
  }
}

void drawRectOutline(int x, int y, int w, int h, uint16_t color) {
  fillRect(x, y, w, 1, color);
  fillRect(x, y + h - 1, w, 1, color);
  fillRect(x, y, 1, h, color);
  fillRect(x + w - 1, y, 1, h, color);
}

void initST7735() {
  pinMode(TFT_CS, OUTPUT); pinMode(TFT_RST, OUTPUT); pinMode(TFT_DC, OUTPUT);
  digitalWrite(TFT_RST, HIGH); delay(10);
  digitalWrite(TFT_RST, LOW);  delay(20);
  digitalWrite(TFT_RST, HIGH); delay(150);
  writeCommand(ST7735_SWRESET); delay(150);
  writeCommand(ST7735_SLPOUT);  delay(200);
  writeCommand(ST7735_COLMOD);  writeData(0x05);
  writeCommand(ST7735_MADCTL);  writeData(0x00);
  writeCommand(ST7735_DISPON);  delay(100);
}