# Gerencia tokens OAuth, lê dados de reprodução e envia comandos de play, pause e volume.

import time
import requests
import re
import config
from bmo_bridge import enviar_rpc

token_spotify_atual = None
token_expira_em = 0
ultima_musica_tocada = ""

def obter_token_acesso_spotify():
    global token_spotify_atual, token_expira_em
    if token_spotify_atual and time.time() < token_expira_em:
        return token_spotify_atual
    try:
        url = "https://accounts.spotify.com/api/token"
        dados = {
            "grant_type": "refresh_token",
            "refresh_token": config.SPOTIFY_REFRESH_TOKEN,
            "client_id": config.SPOTIFY_CLIENT_ID,
            "client_secret": config.SPOTIFY_CLIENT_SECRET,
        }
        res = requests.post(url, data=dados, timeout=5)
        if res.status_code == 200:
            d = res.json()
            token_spotify_atual = d.get("access_token")
            token_expira_em = time.time() + d.get("expires_in", 3600) - 60
            return token_spotify_atual
    except Exception:
        pass
    return None

def obter_dados_player_spotify():
    token = obter_token_acesso_spotify()
    if not token:
        return None
    try:
        url = "https://api.spotify.com/v1/me/player"
        headers = {"Authorization": f"Bearer {token}"}
        res = requests.get(url, headers=headers, timeout=4)
        if res.status_code != 200 or not res.text:
            return None

        dados = res.json()
        tocando = bool(dados.get("is_playing"))
        vol = dados.get("device", {}).get("volume_percent", 100)
        if vol is None:
            vol = 100

        item = dados.get("item")
        if not item:
            return None

        nome = item.get("name", "")
        artistas = ", ".join([a.get("name", "") for a in item.get("artists", [])])
        prog = (dados.get("progress_ms") or 0) // 1000
        dur = (item.get("duration_ms") or 0) // 1000
        return {
            "nome": nome,
            "artista": artistas,
            "progresso": prog,
            "duracao": dur,
            "tocando": tocando,
            "volume": vol,
        }
    except Exception:
        return None

def pausar_spotify():
    token = obter_token_acesso_spotify()
    if not token: return False
    try:
        url = "https://api.spotify.com/v1/me/player/pause"
        headers = {"Authorization": f"Bearer {token}"}
        res = requests.put(url, headers=headers, timeout=4)
        return res.status_code in [200, 204]
    except Exception:
        return False

def reproduzir_spotify():
    token = obter_token_acesso_spotify()
    if not token: return False
    try:
        url = "https://api.spotify.com/v1/me/player/play"
        headers = {"Authorization": f"Bearer {token}"}
        res = requests.put(url, headers=headers, timeout=4)
        return res.status_code in [200, 204]
    except Exception:
        return False

def alterar_volume_spotify(volume_percent):
    token = obter_token_acesso_spotify()
    if not token: return False
    try:
        url = f"https://api.spotify.com/v1/me/player/volume?volume_percent={volume_percent}"
        headers = {"Authorization": f"Bearer {token}"}
        res = requests.put(url, headers=headers, timeout=4)
        return res.status_code in [200, 204]
    except Exception:
        return False

def extrair_comando_volume(texto):
    t = texto.lower()
    if any(k in t for k in ["volume", "som", "altura"]):
        if any(v in t for v in ["aument", "diminu", "coloca", "muda", "poe", "põe", "bota", "ajusta", "seta", "deixa", "vai para", "vai pra"]):
            numeros = re.findall(r"\d+", t)
            if numeros:
                return min(max(int(numeros[0]), 0), 100)
            if "maximo" in t or "máximo" in t or "cem" in t: return 100
            if "metade" in t or "medio" in t or "médio" in t: return 50
            if "mudo" in t or "zero" in t: return 0
    return None

def remover_termo_volume(texto):
    padrao = r"(por\s+favor\s*)?(aument[a-z]*|diminu[a-z]*|coloc[a-z]*|mud[a-z]*|p[oõ]e[a-z]*|bot[a-z]*|ajust[a-z]*|set[a-z]*|deix[a-z]*)\s+(o\s+)?(volume|som|altura)(\s+(do\s+spotify|para|pra|em))?\s*(\d+%?|m[aá]ximo|metade|m[eé]dio|mudo|zero)?(\s*(e|aí|então)\s*)?"
    res = re.sub(padrao, "", texto, flags=re.IGNORECASE).strip()
    return re.sub(r"^(e|aí|então|também|tambem)\s+", "", res, flags=re.IGNORECASE).strip()

def eh_comando_controle_musica(texto):
    t = texto.lower()
    comandos_pause = ["pause", "pausar", "parar", "pausa"]
    if any(p in t for p in comandos_pause): return "pause"
    comandos_play = ["toque", "tocar", "reproduzir", "reproduza", "play", "despausar", "continua", "toca"]
    if any(p in t for p in comandos_play): return "play"
    return None

def eh_pergunta_musica(texto):
    t = texto.lower()
    palavras = ["o que esta tocando", "modo dj", "musica", "spotify", "o que está tocando"]
    return any(p in t for p in palavras)

def thread_monitor_spotify():
    global ultima_musica_tocada
    while True:
        try:
            if config.modo_dj_ativo and not config.processando_ia:
                info = obter_dados_player_spotify()
                if info:
                    enviar_rpc(
                        "atualizarSpotify",
                        [
                            info["nome"], info["artista"], info["progresso"],
                            info["duracao"], info["tocando"], info["volume"],
                        ],
                    )
                    ultima_musica_tocada = info["nome"]
                else:
                    enviar_rpc("atualizarSpotify", ["", "", 0, 0, False, 0])
        except Exception:
            pass
        finally:
            time.sleep(5.0)