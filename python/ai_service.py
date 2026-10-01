# Faz o streaming da resposta do Ollama e gerencia a máquina de estados do Bbmo_bridge

import json
import time
import requests
import config
from bmo_bridge import bmo_set_modo, bmo_enviar_texto_scroll, enviar_rpc
from text_utils import expandir_siglas, limpar_texto, wrap_texto
from spotify_service import (
    obter_dados_player_spotify, pausar_spotify, reproduzir_spotify,
    alterar_volume_spotify, extrair_comando_volume, remover_termo_volume,
    eh_comando_controle_musica, eh_pergunta_musica
)

def worker_fila_bmo():
    while True:
        item_fila = config.fila_pedidos.get()
        prompt, cfg = item_fila
        config.processando_ia = True

        try:
            novo_volume = extrair_comando_volume(prompt)
            if novo_volume is not None:
                alterar_volume_spotify(novo_volume)

            comando = eh_comando_controle_musica(prompt)

            if comando == "pause":
                pausar_spotify()
                info = obter_dados_player_spotify()
                vol_atual = novo_volume if novo_volume is not None else (info["volume"] if info else 100)
                config.modo_dj_ativo = True
                bmo_set_modo(3)
                if info:
                    enviar_rpc("atualizarSpotify", [
                        info["nome"], info["artista"], info["progresso"], info["duracao"], False, vol_atual
                    ])
                else:
                    enviar_rpc("atualizarSpotify", ["Pausado", "", 0, 0, False, vol_atual])

            elif comando == "play":
                reproduzir_spotify()
                info = obter_dados_player_spotify()
                vol_atual = novo_volume if novo_volume is not None else (info["volume"] if info else 100)
                config.modo_dj_ativo = True
                bmo_set_modo(3)
                if info:
                    enviar_rpc("atualizarSpotify", [
                        info["nome"], info["artista"], info["progresso"], info["duracao"], True, vol_atual
                    ])
                else:
                    enviar_rpc("atualizarSpotify", ["Tocando", "", 0, 0, True, vol_atual])

            elif eh_pergunta_musica(prompt):
                info = obter_dados_player_spotify()
                vol_atual = novo_volume if novo_volume is not None else (info["volume"] if info else 100)
                if info and info["tocando"]:
                    config.modo_dj_ativo = True
                    bmo_set_modo(3)
                    enviar_rpc("atualizarSpotify", [
                        info["nome"], info["artista"], info["progresso"], info["duracao"], True, vol_atual
                    ])
                else:
                    config.modo_dj_ativo = False
                    bmo_set_modo(0)
                    bmo_enviar_texto_scroll("Nenhuma musica\ntocando agora.")

            elif novo_volume is not None and len(remover_termo_volume(prompt)) < 4:
                info = obter_dados_player_spotify()
                if info and info["tocando"]:
                    config.modo_dj_ativo = True
                    bmo_set_modo(3)
                    enviar_rpc("atualizarSpotify", [
                        info["nome"], info["artista"], info["progresso"], info["duracao"], info["tocando"], novo_volume
                    ])
                else:
                    bmo_enviar_texto_scroll(f"Volume: {novo_volume}%.")
                    time.sleep(2.5)
                    bmo_set_modo(0)

            else:
                prompt_ia = remover_termo_volume(prompt) if novo_volume is not None else prompt
                config.modo_dj_ativo = False
                linhas_p = wrap_texto(prompt_ia, 20)
                prefixo_display = "P:\n" + "\n".join(linhas_p) + "\n\n"

                bmo_set_modo(1)
                bmo_enviar_texto_scroll(prefixo_display + "Pensando")

                pergunta_processada = expandir_siglas(prompt_ia)
                payload = {
                    "model": config.MODEL,
                    "messages": [
                        {"role": "system", "content": cfg["system_prompt"]},
                        {"role": "user", "content": pergunta_processada},
                    ],
                    "stream": True,
                    "think": False,
                    "keep_alive": config.KEEP_ALIVE,
                    "options": {
                        "num_thread": config.NUM_THREAD,
                        "temperature": cfg["temperature"],
                        "top_p": cfg["top_p"],
                        "repeat_penalty": cfg["repeat_penalty"],
                        "num_predict": cfg["num_predict"],
                        "num_ctx": cfg["num_ctx"],
                    },
                }

                res = requests.post(config.OLLAMA_URL, json=payload, stream=True, timeout=50)
                primeiro_token = True
                resposta_acumulada = ""
                ultimo_push = time.time()

                for line in res.iter_lines():
                    if not line:
                        continue
                    try:
                        chunk_json = json.loads(line)
                    except Exception:
                        continue

                    token = chunk_json.get("message", {}).get("content", "")
                    resposta_acumulada += token

                    if primeiro_token and token.strip():
                        bmo_set_modo(2)
                        primeiro_token = False

                    agora = time.time()
                    if agora - ultimo_push >= 0.35:
                        ultimo_push = agora
                        resp_limpa = limpar_texto(resposta_acumulada)
                        linhas_r = wrap_texto(resp_limpa, 20)
                        texto_frame = prefixo_display + "R:\n" + "\n".join(linhas_r)
                        bmo_enviar_texto_scroll(texto_frame)

                    if chunk_json.get("done", False):
                        break

                try:
                    res.close()
                except Exception:
                    pass

                resp_final = limpar_texto(resposta_acumulada)
                linhas_final = wrap_texto(resp_final, 20)
                texto_concluido = prefixo_display + "R:\n" + "\n".join(linhas_final)
                bmo_enviar_texto_scroll(texto_concluido)

                time.sleep(1.0)
                bmo_set_modo(0)

        except Exception:
            bmo_set_modo(0)
            bmo_enviar_texto_scroll("Erro ao processar.")
            time.sleep(2.0)
        finally:
            config.processando_ia = False
            config.fila_pedidos.task_done()