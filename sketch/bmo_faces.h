#pragma once
#include "tft_st7735.h"

unsigned long ultimo_tempo_standby = 0;
int standby_estado = 0;
int standby_frame = 0;
unsigned long duracao_fase_standby = 2500;

void limparTopoLivreDeLixo() {
  fillRect(0, 0, 128, 65, BMO_TEAL);
  fillRect(0, 65, 128, 2, DARK_TEAL);
}

void limparAreaOlhos() {
  fillRect(26, 18, 24, 16, BMO_TEAL);
  fillRect(78, 18, 24, 16, BMO_TEAL);
}

void limparAreaBoca() {
  fillRect(20, 34, 88, 26, BMO_TEAL);
}

void desenharOlhoNormal(int cx, int cy, int offset_x = 0) {
  fillCircle(cx + offset_x, cy, 4, BLACK);
  drawPixel(cx + offset_x - 1, cy - 1, WHITE);
}

void desenharOlhoPiscando(int cx, int cy) {
  fillRect(cx - 4, cy, 9, 2, BLACK);
}

void desenharOlhoFelizArco(int cx, int cy) {
  fillRect(cx - 4, cy + 1, 2, 2, BLACK);
  fillRect(cx - 2, cy - 1, 2, 2, BLACK);
  fillRect(cx, cy - 2, 2, 2, BLACK);
  fillRect(cx + 2, cy - 1, 2, 2, BLACK);
  fillRect(cx + 4, cy + 1, 2, 2, BLACK);
}

void desenharSorrisoPadrao() {
  for (int dx = -17; dx <= 17; dx++) {
    int dy = ((17 * 17 - dx * dx) * 9) / 289;
    fillRect(64 + dx, 36 + dy, 1, 3, BLACK);
  }
}

void desenharSorrisoAbertoFeliz() {
  for (int dx = -14; dx <= 14; dx++) {
    int dyMax = ((14 * 14 - dx * dx) * 10) / 196;
    fillRect(64 + dx, 38, 1, max(dyMax, 2), BLACK);
  }
  fillRect(60, 38, 8, 3, WHITE);
  fillCircle(24, 37, 4, CANDY_PINK);
  fillCircle(104, 37, 4, CANDY_PINK);
}

void desenharSorrisoSapeca(bool ladoDireito) {
  for (int dx = -15; dx <= 15; dx++) {
    int baseDy = ((15 * 15 - dx * dx) * 8) / 225;
    int incl = ladoDireito ? (dx / 3) : (-dx / 3);
    fillRect(64 + dx, 38 + baseDy - incl, 1, 3, BLACK);
  }
  if (ladoDireito) fillCircle(104, 36, 4, CANDY_PINK);
  else fillCircle(24, 36, 4, CANDY_PINK);
}

void desenharRostoFeliz() {
  limparTopoLivreDeLixo();
  desenharOlhoNormal(38, 26);
  desenharOlhoNormal(90, 26);
  desenharSorrisoPadrao();
}

void prepararRostoPensando() {
  limparTopoLivreDeLixo();
  fillRect(25, 18, 26, 4, BLACK);
  fillRect(25, 28, 26, 4, BLACK);
  fillRect(77, 18, 26, 4, BLACK);
  fillRect(77, 28, 26, 4, BLACK);

  for (int dx = -14; dx <= 14; dx++) {
    int dy = (dx * dx * 6) / 196;
    fillRect(64 + dx, 44 + dy, 1, 2, BLACK);
  }
}

void atualizarOlhosPensando(int frame) {
  int offset = (frame == 0) ? -6 : (frame == 1) ? -2 : (frame == 2) ? 2 : 6;
  fillRect(26, 21, 24, 9, BMO_TEAL);
  fillRect(78, 21, 24, 9, BMO_TEAL);
  fillCircle(38 + offset, 25, 4, BLACK);
  fillCircle(90 + offset, 25, 4, BLACK);
  drawPixel(37 + offset, 24, WHITE);
  drawPixel(89 + offset, 24, WHITE);
}

void desenharBaseFalando() {
  limparTopoLivreDeLixo();
  desenharOlhoNormal(38, 26);
  desenharOlhoNormal(90, 26);
}

void animarBocaFalando(int estado) {
  limparAreaBoca();
  if (estado == 0) {
    fillCircle(64, 46, 7, BLACK);
    fillRect(58, 41, 12, 3, WHITE);
  } else if (estado == 1) {
    fillRect(54, 44, 20, 6, BLACK);
    fillRect(58, 44, 12, 2, WHITE);
  } else {
    desenharSorrisoPadrao();
  }
}

void atualizarAnimacoesStandby() {
  if (millis() - ultimo_tempo_standby > duracao_fase_standby) {
    ultimo_tempo_standby = millis();

    switch (standby_estado) {
      case 0:
        standby_frame = 0;
        standby_estado = (rand() % 4) + 1;
        duracao_fase_standby = 50;
        break;

      case 1: // Piscar
        if (standby_frame == 0) {
          limparAreaOlhos();
          desenharOlhoPiscando(38, 26); desenharOlhoPiscando(90, 26);
          duracao_fase_standby = 130; standby_frame = 1;
        } else {
          limparAreaOlhos();
          desenharOlhoNormal(38, 26); desenharOlhoNormal(90, 26);
          standby_estado = 0; duracao_fase_standby = 2500 + (rand() % 1500);
        }
        break;

      case 2: // Olhar pros lados
        if (standby_frame == 0) {
          int offset = (rand() % 2 == 0) ? -3 : 3;
          limparAreaOlhos();
          desenharOlhoNormal(38, 26, offset); desenharOlhoNormal(90, 26, offset);
          duracao_fase_standby = 800; standby_frame = 1;
        } else {
          limparAreaOlhos();
          desenharOlhoNormal(38, 26); desenharOlhoNormal(90, 26);
          standby_estado = 0; duracao_fase_standby = 2500 + (rand() % 1500);
        }
        break;

      case 3: // Piscadinha
        if (standby_frame == 0) {
          limparAreaOlhos(); limparAreaBoca();
          desenharOlhoNormal(38, 26); desenharOlhoFelizArco(90, 26);
          desenharSorrisoSapeca(true);
          duracao_fase_standby = 650; standby_frame = 1;
        } else {
          limparAreaOlhos(); limparAreaBoca();
          desenharOlhoNormal(38, 26); desenharOlhoNormal(90, 26);
          desenharSorrisoPadrao();
          standby_estado = 0; duracao_fase_standby = 2500 + (rand() % 1500);
        }
        break;

      case 4: // Sorriso aberto
        if (standby_frame == 0) {
          limparAreaOlhos(); limparAreaBoca();
          desenharOlhoFelizArco(38, 26); desenharOlhoFelizArco(90, 26);
          desenharSorrisoAbertoFeliz();
          duracao_fase_standby = 850; standby_frame = 1;
        } else {
          limparAreaOlhos(); limparAreaBoca();
          desenharOlhoNormal(38, 26); desenharOlhoNormal(90, 26);
          desenharSorrisoPadrao();
          standby_estado = 0; duracao_fase_standby = 3000 + (rand() % 1500);
        }
        break;
    }
  }
}