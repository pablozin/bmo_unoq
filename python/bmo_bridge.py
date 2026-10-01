#Cuida de empacotar o MessagePack e enviar comandos pelo socket Unix para a tela.
import socket
import time
import msgpack
import config

def enviar_rpc(metodo, params):
    try:
        packet = msgpack.packb([0, 1, metodo, params], use_bin_type=True)
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
            sock.settimeout(2.5)
            sock.connect(config.SOCKET_PATH)
            sock.sendall(packet)
            try:
                sock.recv(128)
            except socket.timeout:
                pass
        return True
    except Exception:
        return False

def bmo_set_modo(modo):
    enviar_rpc("bmoModo", [modo])

def bmo_enviar_texto_scroll(texto_completo):
    chunk_size = 120
    chunks = [
        texto_completo[i : i + chunk_size]
        for i in range(0, len(texto_completo), chunk_size)
    ]
    try:
        with socket.socket(socket.AF_UNIX, socket.SOCK_STREAM) as sock:
            sock.settimeout(2.5)
            sock.connect(config.SOCKET_PATH)
            for i, chunk in enumerate(chunks):
                limpar = (i == 0)
                terminar = (i == len(chunks) - 1)
                packet = msgpack.packb([0, 1, "exibirTextoAvancado", [chunk, limpar, terminar]], use_bin_type=True)
                sock.sendall(packet)
                try:
                    sock.recv(128)
                except socket.timeout:
                    pass
                time.sleep(0.015)
    except Exception:
        pass