# Higieniza comandos matemáticos, remove caracteres indesejados e faz o wrap para 20 colunas
import re

SIGLAS_HARDWARE = {
    r"\bula\b": "ULA (Unidade Logica e Aritmetica)",
    r"\buc\b": "UC (Unidade de Controle)",
    r"\bcpu\b": "CPU (Processador)",
    r"\bram\b": "RAM (Memoria Principal)",
    r"\brom\b": "ROM (Memoria de Leitura)",
}

UNICODE_MATH_MAP = {
    "∫": "integral de", "²": "^2", "³": "^3", "π": "pi",
    "θ": "teta", "±": "+/-", "≈": "~=", "∞": "infinito",
    "√": "raiz de", "∑": "soma", "Δ": "delta", "≤": "<=", "≥": ">=",
    "“": '"', "”": '"', "’": "'",
}

RE_UNICODE_MATH = re.compile("|".join(re.escape(k) for k in UNICODE_MATH_MAP.keys()))
RE_LATEX_COMMANDS = re.compile(r"\\[a-zA-Z]+|\\[\[\]\(\)\{\};!,]|\\frac|\\int")
RE_MARKDOWN = re.compile(r"[*#$`]")
RE_MULTI_SPACES = re.compile(r"\s+")
RE_CLEAN_CHARS = re.compile(r"[^a-zA-Z0-9\s\.,!\?'\"\(\)\+\-\*/=<>\[\]\^:;%_áéíóúâêôãõçÁÉÍÓÚÂÊÔÃÕÇ]")

def expandir_siglas(texto):
    t = texto
    for sigla, expansao in SIGLAS_HARDWARE.items():
        t = re.sub(sigla, expansao, t, flags=re.IGNORECASE)
    return t

def limpar_texto(texto):
    t = RE_UNICODE_MATH.sub(lambda m: UNICODE_MATH_MAP[m.group(0)], texto)
    t = t.replace("{", "(").replace("}", ")")
    t = RE_LATEX_COMMANDS.sub(" ", t)
    t = RE_MARKDOWN.sub("", t)
    t = RE_CLEAN_CHARS.sub(" ", t)
    return RE_MULTI_SPACES.sub(" ", t).strip()

def wrap_texto(texto, largura=20):
    palavras = texto.split()
    linhas, atual = [], ""
    for palavra in palavras:
        candidato = palavra if not atual else atual + " " + palavra
        if len(candidato) <= largura:
            atual = candidato
        else:
            linhas.append(atual)
            atual = palavra
    if atual:
        linhas.append(atual)
    return linhas