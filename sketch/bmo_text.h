//Cuida do parsing de UTF-8, acentuação e do algoritmo de scroll

#pragma once
#include "tft_st7735.h"
#include "font5x7.h"

String texto_display = "";
int texto_total_h = 0;
float scroll_y = 0;
float scroll_dir = 1.0;
int scroll_pause = 35;
unsigned long ultimo_scroll_ms = 0;

const uint8_t font_pt[][5] = {
  {0x20, 0x56, 0x55, 0x54, 0x78}, {0x20, 0x55, 0x56, 0x54, 0x78},
  {0x20, 0x55, 0x56, 0x55, 0x78}, {0x20, 0x56, 0x55, 0x56, 0x78},
  {0x38, 0x56, 0x55, 0x54, 0x18}, {0x38, 0x56, 0x55, 0x56, 0x18},
  {0x00, 0x46, 0x7D, 0x40, 0x00}, {0x38, 0x46, 0x45, 0x44, 0x38},
  {0x38, 0x46, 0x45, 0x46, 0x38}, {0x38, 0x45, 0x46, 0x45, 0x38},
  {0x3C, 0x42, 0x41, 0x20, 0x7C}, {0x38, 0x44, 0xC4, 0x44, 0x20}
};

uint8_t decodificarUtf8(uint8_t c2) {
  switch (c2) {
    case 0xA1: case 0x81: return 128; case 0xA0: case 0x80: return 129;
    case 0xA3: case 0x83: return 130; case 0xA2: case 0x82: return 131;
    case 0xA9: case 0x89: return 132; case 0xAA: case 0x8A: return 133;
    case 0xAD: case 0x8D: return 134; case 0xB3: case 0x93: return 135;
    case 0xB4: case 0x94: return 136; case 0xB5: case 0x95: return 137;
    case 0xBA: case 0x9A: return 138; case 0xA7: case 0x87: return 139;
    default: return '?';
  }
}

String preDecodificarUtf8(const String &entrada) {
  String saida = "";
  int len = entrada.length();
  saida.reserve(len);
  for (int i = 0; i < len; i++) {
    uint8_t c = (uint8_t)entrada[i];
    if (c == 0xC3 && (i + 1) < len) {
      i++; saida += (char)decodificarUtf8((uint8_t)entrada[i]);
    } else {
      saida += (char)c;
    }
  }
  return saida;
}

void drawCharFast(int x, int y, uint8_t c, uint16_t color, uint16_t bg) {
  const uint8_t* charData = NULL;
  if (c >= 32 && c <= 126) charData = &font5x7[(c - 32) * 5];
  else if (c >= 128 && c <= 139) charData = font_pt[c - 128];
  else return;

  setAddrWindow(x, y, x + 4, y + 7);
  digitalWrite(TFT_DC, HIGH); digitalWrite(TFT_CS, LOW);
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 5; col++) {
      if (charData[col] & (1 << row)) {
        SPI.transfer(color >> 8); SPI.transfer(color & 0xFF);
      } else {
        SPI.transfer(bg >> 8); SPI.transfer(bg & 0xFF);
      }
    }
  }
  digitalWrite(TFT_CS, HIGH);
}

void drawCharScroll(int x, int y, uint8_t c, uint16_t color, uint16_t bg) {
  const uint8_t* charData = NULL;
  if (c >= 32 && c <= 126) charData = &font5x7[(c - 32) * 5];
  else if (c >= 128 && c <= 139) charData = font_pt[c - 128];
  else return;

  int y0 = max(y, 67); int y1 = min(y + 9, 159);
  if (y0 > y1) return;

  setAddrWindow(x, y0, x + 5, y1);
  digitalWrite(TFT_DC, HIGH); digitalWrite(TFT_CS, LOW);

  for (int row = y0 - y; row <= y1 - y; row++) {
    for (int col = 0; col < 6; col++) {
      if (row < 8 && col < 5 && (charData[col] & (1 << row))) {
        SPI.transfer(color >> 8); SPI.transfer(color & 0xFF);
      } else {
        SPI.transfer(bg >> 8); SPI.transfer(bg & 0xFF);
      }
    }
  }
  digitalWrite(TFT_CS, HIGH);
}

void drawScrollLine(const String &texto, int startIdx, int endIdx, int curY) {
  int y0 = max(curY, 67); int y1 = min(curY + 9, 159);
  if (y0 > y1) return;

  int charCount = endIdx - startIdx;
  int textW = charCount * 6;
  int startX = max((128 - textW) / 2, 0);

  if (startX > 0) fillRect(0, y0, startX, y1 - y0 + 1, BMO_TEAL);

  int curX = startX;
  for (int i = startIdx; i < endIdx; i++) {
    uint8_t c = (uint8_t)texto[i];
    if (curX <= 122) {
      drawCharScroll(curX, curY, c, BLACK, BMO_TEAL);
      curX += 6;
    }
  }
  if (curX < 128) fillRect(curX, y0, 128 - curX, y1 - y0 + 1, BMO_TEAL);
}

void limparAreaTexto() {
  fillRect(0, 67, 128, 93, BMO_TEAL);
}

void desenharPaginaTextoScroll() {
  int curY = 67 - (int)scroll_y;
  int startIdx = 0;
  int last_y = 67;
  int totalLen = texto_display.length();

  while (startIdx < totalLen) {
    int endIdx = texto_display.indexOf('\n', startIdx);
    if (endIdx == -1) endIdx = totalLen;

    if (curY + 9 >= 67 && curY <= 159) {
      drawScrollLine(texto_display, startIdx, endIdx, curY);
    }
    curY += 10;
    last_y = curY;
    startIdx = endIdx + 1;
    if (curY > 160) break;
  }

  if (last_y < 160) {
    fillRect(0, max(last_y, 67), 128, 160 - max(last_y, 67), BMO_TEAL);
  }
}

void atualizarTextoPensando(int frame) {
  int curY = 67 - (int)scroll_y;
  int startIdx = 0;
  int totalLen = texto_display.length();

  while (startIdx < totalLen) {
    int endIdx = texto_display.indexOf('\n', startIdx);
    if (endIdx == -1) endIdx = totalLen;

    if (endIdx - startIdx >= 8 && texto_display.startsWith("Pensando", startIdx)) {
      int y0 = max(curY, 67); int y1 = min(curY + 9, 159);
      if (y0 <= y1) {
        char anim[12] = "Pensando";
        int lenAnim = 8;
        if (frame >= 1) anim[lenAnim++] = '.';
        if (frame >= 2) anim[lenAnim++] = '.';
        if (frame >= 3) anim[lenAnim++] = '.';
        anim[lenAnim] = '\0';

        int startX = (128 - (11 * 6)) / 2;
        if (startX > 0) fillRect(0, y0, startX, y1 - y0 + 1, BMO_TEAL);

        int curX = startX;
        for (int i = 0; i < lenAnim; i++) {
          drawCharScroll(curX, curY, anim[i], BLACK, BMO_TEAL);
          curX += 6;
        }
        if (curX < 128) fillRect(curX, y0, 128 - curX, y1 - y0 + 1, BMO_TEAL);
      }
      break;
    }
    curY += 10;
    startIdx = endIdx + 1;
  }
}

void printTextoCentralizado(const String &texto, int curY, uint16_t cor, uint16_t bg) {
  int len = texto.length();
  int startX = max((128 - (len * 6)) / 2, 2);
  int curX = startX;

  for (int i = 0; i < len; i++) {
    uint8_t c = (uint8_t)texto[i];
    if (curX <= 122) {
      drawCharFast(curX, curY, c, cor, bg);
      curX += 6;
    }
  }
}