import threading
from flask import Flask, render_template_string, request
import config
from spotify_service import thread_monitor_spotify
from ai_service import worker_fila_bmo

app = Flask(__name__)

MINIMAL_HTML = """<!DOCTYPE html>
<html>
<head><meta charset="utf-8"><title>BMO Config</title></head>
<body style="font-family: monospace; max-width: 600px; margin: 20px auto; padding: 0 10px;">
    <h2>BMO Controller</h2>
    <form method="POST">
        <fieldset>
            <legend>Parâmetros da IA</legend>
            <p><label>System Prompt:</label><br><textarea name="system_prompt" rows="4" style="width: 100%;">{{ config.system_prompt }}</textarea></p>
            <p><label>Temperature:</label> <input type="number" step="0.05" min="0" max="2" name="temperature" value="{{ config.temperature }}">
            <label>Top P:</label> <input type="number" step="0.05" min="0" max="1" name="top_p" value="{{ config.top_p }}"></p>
            <p><label>Repeat Penalty:</label> <input type="number" step="0.05" min="0" max="2" name="repeat_penalty" value="{{ config.repeat_penalty }}"></p>
            <p><label>Num Predict:</label> <input type="number" name="num_predict" value="{{ config.num_predict }}">
            <label>Num Ctx:</label> <input type="number" name="num_ctx" value="{{ config.num_ctx }}"></p>
        </fieldset><br>
        <fieldset>
            <legend>Comando / Pergunta</legend>
            <p><input type="text" name="prompt" placeholder="Digite sua pergunta..." autofocus required style="width: 100%; box-sizing: border-box; padding: 8px;"></p>
            <p><button type="submit" style="padding: 8px 16px;">Enviar</button></p>
        </fieldset>
    </form>
</body>
</html>"""

@app.route("/", methods=["GET", "POST"])
def index():
    if request.method == "POST":
        config.config_padrao["system_prompt"] = request.form.get("system_prompt", config.config_padrao["system_prompt"]).strip()
        config.config_padrao["temperature"] = float(request.form.get("temperature", config.config_padrao["temperature"]))
        config.config_padrao["top_p"] = float(request.form.get("top_p", config.config_padrao["top_p"]))
        config.config_padrao["repeat_penalty"] = float(request.form.get("repeat_penalty", config.config_padrao["repeat_penalty"]))
        config.config_padrao["num_predict"] = int(request.form.get("num_predict", config.config_padrao["num_predict"]))
        config.config_padrao["num_ctx"] = int(request.form.get("num_ctx", config.config_padrao["num_ctx"]))

        user_prompt = request.form.get("prompt", "").strip()
        if user_prompt:
            config.fila_pedidos.put((user_prompt, dict(config.config_padrao)))

    return render_template_string(MINIMAL_HTML, config=config.config_padrao)

if __name__ == "__main__":
    threading.Thread(target=thread_monitor_spotify, daemon=True).start()
    threading.Thread(target=worker_fila_bmo, daemon=True).start()
    app.run(host="0.0.0.0", port=5000, debug=False)