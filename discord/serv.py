from sanic import Sanic, response
from sanic.request import Request
import os
import socket

app = Sanic("FileUploadServer")

def get_local_ip():
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return "127.0.0.1"

@app.route("/upload", methods=["POST"])
async def upload(request: Request):
    file = request.files.get("file")
    if file is None:
        return response.json({"error": "Aucun fichier fourni"}, status=400)
    
    # Prendre le nom du fichier reçu
    filename = file.name
    client_ip = request.remote_addr or "unknown"
    
    # Créer un dossier pour chaque client IP
    save_dir = os.path.join(os.getcwd(), client_ip)
    os.makedirs(save_dir, exist_ok=True)
    
    # Enregistrer le fichier dans ce dossier
    filepath = os.path.join(save_dir, filename)
    with open(filepath, "wb") as f:
        f.write(file.body)
    
    print(f"✅ Fichier sauvegardé : {filepath}")
    return response.json({"message": f"Fichier sauvegardé sous {filepath}"})

if __name__ == '__main__':
    local_ip = get_local_ip()
    port = 5050
    print(f"🌐 Le client doit se connecter à : {local_ip}:{port}")
    app.run(host="0.0.0.0", port=port)
