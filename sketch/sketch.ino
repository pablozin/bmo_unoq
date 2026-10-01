#include <Arduino_RouterBridge.h>
#include "tft_st7735.h"
#include "bmo_text.h"
#include "bmo_faces.h"
#include "bmo_dj.h"

int bmo_modo = 0;
static int ultimo_modo = 0;

unsigned long ultimo_tempo_anim = 0;
unsigned long ultimo_tick_relogio = 0;
int frame_anim = 0;

bool exibirTextoAvancado(String chunk, bool limpar, bool terminar) {
  if (limpar) {
    texto_display = "";
    scroll_y = 0;
    scroll_dir = 1.0;
    scroll_pause = 35;
  }
  texto_display += preDecodificarUtf8(chunk);

  if (terminar) {
    int linhas = 1;
    int len = texto_display.length();
    for (int i = 0; i < len; i++) {
      if (texto_display[i] == '\n') linhas++;
    }
    texto_total_h = linhas * 10;
    
    if (bmo_modo == 1 || bmo_modo == 2) {
      scroll_y = max(texto_total_h - 85, 0);
    }
    desenharPaginaTextoScroll();
  }
  return true;
}

bool bmoModo(int modo) {
  int modo_anterior = bmo_modo;
  bmo_modo = modo;

  if (modo == 0) {
    musica_tocando = false;
    desenharRostoFeliz();
    if (texto_display.length() > 0) {
      desenharPaginaTextoScroll();
    }
    standby_estado = 0;
    standby_frame = 0;
    ultimo_tempo_standby = millis();
    duracao_fase_standby = 2500;
    return true;
  }

  if (modo == 1 && modo_anterior != 1) {
    limparAreaTexto();
    texto_display = "";
    texto_total_h = 0;
    scroll_y = 0;
  }

  if (modo_anterior == 3 && modo != 3) {
    limparAreaTexto();
    texto_display = "";
    texto_total_h = 0;
    scroll_y = 0;
  }

  ultimo_modo = modo;

  if (modo != 3) {
    musica_tocando = false;
    if (modo == 1) {
      prepararRostoPensando();
      frame_anim = 0;
      atualizarOlhosPensando(0);
    } else if (modo == 2) {
      desenharBaseFalando();
      frame_anim = 0;
      animarBocaFalando(0);
    }
    if (texto_display.length() > 0) {
      desenharPaginaTextoScroll();
    }
  } else if (modo == 3) {
    texto_display = "";
    texto_total_h = 0;
    scroll_y = 0;
    frame_anim = 0;
    fillScreen(BMO_TEAL);
    desenharFitaCasseteEstatica();
    desenharBadgeStatus(musica_tocando);
    desenharIconePlayPauseDireita(musica_tocando);
    animarCarreteisFita(0);
    desenharInfoMusicaCompleta();
    desenharPlayerMusica();
  }
  return true;
}

bool atualizarSpotify(String nome, String artista, int progresso, int duracao, bool tocando, int volume) {
  String nomeLimpo = preDecodificarUtf8(nome);
  String artistaLimpo = preDecodificarUtf8(artista);

  bool nome_mudou = (musica_nome != nomeLimpo);
  musica_nome = nomeLimpo;
  musica_artista = artistaLimpo;
  musica_progresso = progresso;
  musica_duracao = duracao;
  musica_tocando = tocando;
  musica_volume = volume;

  if (bmo_modo == 3) {
    desenharBadgeStatus(tocando);
    desenharIconePlayPauseDireita(tocando);
    if (nome_mudou) desenharInfoMusicaCompleta();
    desenharPlayerMusica();
  }
  return true;
}

void setup() {
  SPI.begin();
  SPI.beginTransaction(SPISettings(8000000, MSBFIRST, SPI_MODE0));
  initST7735();
  fillScreen(BMO_TEAL);

  texto_display.reserve(800);

  Bridge.begin();
  Monitor.begin(115200);

  Bridge.provide("exibirTextoAvancado", exibirTextoAvancado);
  Bridge.provide("bmoModo", bmoModo);
  Bridge.provide("atualizarSpotify", atualizarSpotify);

  bmoModo(0);
  exibirTextoAvancado("Estou pronto!", true, true);
}

void loop() {
  if (bmo_modo == 0 && texto_total_h > 85) {
    if (millis() - ultimo_scroll_ms > 35) {
      ultimo_scroll_ms = millis();
      if (scroll_pause > 0) {
        scroll_pause--;
      } else {
        scroll_y += scroll_dir;
        int max_scroll = max(texto_total_h - 85, 0);
        if (scroll_y >= max_scroll) {
          scroll_y = max_scroll; scroll_dir = -1.0; scroll_pause = 40;
        } else if (scroll_y <= 0) {
          scroll_y = 0; scroll_dir = 1.0; scroll_pause = 40;
        }
        desenharPaginaTextoScroll();
      }
    }
  }

  if (bmo_modo == 0) {
    atualizarAnimacoesStandby();
  } else if (bmo_modo == 1) {
    if (millis() - ultimo_tempo_anim > 280) {
      ultimo_tempo_anim = millis();
      atualizarOlhosPensando(frame_anim);
      atualizarTextoPensando(frame_anim);
      frame_anim = (frame_anim + 1) % 4;
    }
  } else if (bmo_modo == 2) {
    if (millis() - ultimo_tempo_anim > 160) {
      ultimo_tempo_anim = millis();
      animarBocaFalando(frame_anim % 3);
      frame_anim++;
    }
  } else if (bmo_modo == 3 && musica_tocando) {
    if (millis() - ultimo_tempo_anim > 250) {
      ultimo_tempo_anim = millis();
      animarCarreteisFita(frame_anim);
      animarVisualizadorChunky();
      frame_anim++;
    }
    if (millis() - ultimo_tick_relogio > 1000) {
      ultimo_tick_relogio = millis();
      musica_progresso++;
      if (musica_progresso <= musica_duracao) desenharPlayerMusica();
    }
    if (musica_nome.length() > 16 && millis() - ultimo_tempo_marquee > 350) {
      ultimo_tempo_marquee = millis();
      if (marquee_pausa > 0) marquee_pausa--;
      else {
        marquee_offset++;
        if (marquee_offset > (int)musica_nome.length() - 14) {
          marquee_offset = 0; marquee_pausa = 6;
        }
        desenharTituloRolante();
      }
    }
  }
}