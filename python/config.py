# Centraliza os caminhos, as credenciais e as variáveis compartilhadas
import os
import queue
from dotenv import load_dotenv

load_dotenv()

SOCKET_PATH = "/run/arduino-router.sock"
OLLAMA_URL = "http://172.17.0.1:11434/api/chat"
MODEL = "qwen3:0.6b"
NUM_THREAD = 4
KEEP_ALIVE = "30m"

fila_pedidos = queue.Queue()
modo_dj_ativo = False
processando_ia = False

SPOTIFY_CLIENT_ID = os.getenv("SPOTIFY_CLIENT_ID", "")
SPOTIFY_CLIENT_SECRET = os.getenv("SPOTIFY_CLIENT_SECRET", "")
SPOTIFY_REFRESH_TOKEN = os.getenv("SPOTIFY_REFRESH_TOKEN", "")

def carregar_system_prompt():
    caminho = os.path.join(os.path.dirname(os.path.abspath(__file__)), "system_prompt.txt")
    if os.path.exists(caminho):
        try:
            with open(caminho, "r", encoding="utf-8") as f:
                return f.read().strip()
        except Exception:
            pass
    return (
        "Você é o BMO, assistente técnico de hardware e ciências. "
        "Responda em português simples, direto e em no máximo 2 frases com ponto final. "
        "Escreva sempre em texto corrido e limpo. "
        "Em cálculos ou fórmulas, use apenas caracteres simples do teclado como +, -, *, /, =."
    )

config_padrao = {
    "system_prompt": carregar_system_prompt(),
    "temperature": 0.2,
    "top_p": 0.9,
    "repeat_penalty": 1.2,
    "num_predict": 180,
    "num_ctx": 512,
}