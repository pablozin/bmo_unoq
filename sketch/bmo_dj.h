//Cuida do tocador retrô, fita cassete, carretéis, equalizador e letreiro rolante.
#pragma once
#include "tft_st7735.h"
#include "bmo_text.h"

String musica_nome = "";
String musica_artista = "";
int musica_progresso = 0;
int musica_duracao = 0;
bool musica_tocando = false;
int musica_volume = 100;

int marquee_offset = 0;
int marquee_pausa = 6;
unsigned long ultimo_tempo_marquee = 0;

int eq_blocos[5] = {2, 4, 6, 3, 5};
int eq_picos[5]  = {2, 4, 6, 3, 5};
int eq_delay[5]  = {0, 0, 0, 0, 0};

void desenharFitaCasseteEstatica() {
  fillRect(24, 6, 80, 42, GADGET_BEIGE);
  drawRectOutline(24, 6, 80, 42, BLACK);
  drawRectOutline(25, 7, 78, 40, BLACK);
  fillRect(30, 10, 68, 14, CANDY_PINK);
  drawRectOutline(30, 10, 68, 14, BLACK);
  fillRect(36, 27, 56, 16, BLACK);
  drawPixel(27, 9, BLACK); drawPixel(100, 9, BLACK);
  drawPixel(27, 42, BLACK); drawPixel(100, 42, BLACK);
}

void animarCarreteisFita(int frame) {
  int r1_x = 48, r2_x = 80; int cy = 35;
  fillRect(r1_x - 5, cy - 5, 11, 11, WHITE); drawRectOutline(r1_x - 5, cy - 5, 11, 11, BLACK);
  fillRect(r2_x - 5, cy - 5, 11, 11, WHITE); drawRectOutline(r2_x - 5, cy - 5, 11, 11, BLACK);
  int offset_x = (frame % 2 == 0) ? -2 : 2; int offset_y = (frame / 2 % 2 == 0) ? -2 : 2;
  drawPixel(r1_x + offset_x, cy + offset_y, BLACK); drawPixel(r1_x - offset_x, cy - offset_y, BLACK);
  drawPixel(r2_x + offset_x, cy + offset_y, BLACK); drawPixel(r2_x - offset_x, cy - offset_y, BLACK);
}

void desenharBadgeStatus(bool tocando) {
  fillRect(4, 4, 16, 44, GADGET_BEIGE); drawRectOutline(4, 4, 16, 44, BLACK);
  if (tocando) {
    fillRect(6, 6, 12, 10, CANDY_YELLOW); drawCharFast(9, 8, 'P', BLACK, CANDY_YELLOW);
  } else {
    fillRect(6, 6, 12, 10, CANDY_PINK); drawCharFast(9, 8, 'S', BLACK, CANDY_PINK);
  }
}

void desenharIconePlayPauseDireita(bool tocando) {
  fillRect(108, 4, 16, 44, GADGET_BEIGE); drawRectOutline(108, 4, 16, 44, BLACK);
  if (tocando) {
    fillRect(110, 6, 12, 12, CANDY_YELLOW); drawRectOutline(110, 6, 12, 12, BLACK);
    for (int i = 0; i < 7; i++) {
      fillRect(113 + i, 12 - i, 1, (i * 2) + 1, BLACK);
    }
  } else {
    fillRect(110, 6, 12, 12, CANDY_PINK); drawRectOutline(110, 6, 12, 12, BLACK);
    fillRect(113, 9, 2, 6, BLACK); fillRect(117, 9, 2, 6, BLACK);
  }
}

void animarVisualizadorChunky() {
  int startX = 14; int baseY = 154; int espaco = 4;
  for (int i = 0; i < 5; i++) {
    eq_blocos[i] = constrain(eq_blocos[i] + (rand() % 3) - 1, 1, 6);
    if (eq_blocos[i] >= eq_picos[i]) {
      eq_picos[i] = eq_blocos[i]; eq_delay[i] = 2;
    } else {
      if (eq_delay[i] > 0) eq_delay[i]--;
      else if (eq_picos[i] > 1) eq_picos[i]--;
    }
    int x = startX + (i * (16 + espaco));
    fillRect(x, baseY - 45, 16, 47, BMO_TEAL);

    for (int b = 0; b < eq_blocos[i]; b++) {
      int yBloco = baseY - (b * 5);
      fillRect(x, yBloco - 4, 16, 4, CANDY_YELLOW);
      drawRectOutline(x, yBloco - 4, 16, 4, BLACK);
    }
    int yPico = baseY - ((eq_picos[i] - 1) * 5);
    fillRect(x, yPico - 4, 16, 4, CANDY_PINK);
    drawRectOutline(x, yPico - 4, 16, 4, BLACK);
  }
}

void desenharTituloRolante() {
  fillRect(6, 56, 116, 15, GADGET_BEIGE); drawRectOutline(6, 56, 116, 15, BLACK);
  int len = musica_nome.length();
  if (len <= 16) {
    printTextoCentralizado(musica_nome, 60, BLACK, GADGET_BEIGE);
    return;
  }
  int maxChars = min(16, len - marquee_offset);
  int curX = 10;
  for (int i = 0; i < maxChars; i++) {
    uint8_t c = (uint8_t)musica_nome[marquee_offset + i];
    if (curX <= 112) {
      drawCharFast(curX, 60, c, BLACK, GADGET_BEIGE);
      curX += 6;
    }
  }
}

void desenharInfoMusicaCompleta() {
  marquee_offset = 0; marquee_pausa = 6;
  desenharTituloRolante();
  fillRect(6, 75, 116, 13, BMO_TEAL);
  printTextoCentralizado(musica_artista, 78, BLACK, BMO_TEAL);
}

void desenharPlayerMusica() {
  if (musica_duracao <= 0) return;
  int p_min = musica_progresso / 60; int p_seg = musica_progresso % 60;
  int d_min = musica_duracao / 60; int d_seg = musica_duracao % 60;
  char buf_tempo[20];
  snprintf(buf_tempo, sizeof(buf_tempo), "%02d:%02d / %02d:%02d", p_min, p_seg, d_min, d_seg);

  fillRect(10, 92, 108, 9, BMO_TEAL);
  printTextoCentralizado(String(buf_tempo), 92, BLACK, BMO_TEAL);

  int barW = 100;
  int progW = constrain(map(musica_progresso, 0, musica_duracao, 0, barW), 0, barW);
  fillRect(14, 104, barW, 8, GADGET_BEIGE);
  drawRectOutline(14, 104, barW, 8, BLACK);
  if (progW > 0) fillRect(15, 105, progW - 1, 6, CANDY_PINK);

  char buf_vol[12];
  snprintf(buf_vol, sizeof(buf_vol), "VOL %d%%", musica_volume);
  fillRect(31, 11, 66, 12, CANDY_PINK);

  int volLen = String(buf_vol).length();
  int startX = 30 + (68 - (volLen * 6)) / 2;
  int curX = startX;
  for (int i = 0; i < volLen; i++) {
    drawCharFast(curX, 14, buf_vol[i], BLACK, CANDY_PINK);
    curX += 6;
  }
}