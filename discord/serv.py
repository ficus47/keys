from sanic import Sanic, response
from sanic.request import Request
import os
import socket

app = Sanic("FileUploadServer")

def get_local_ip():
    """Retourne l'adresse IP locale de la machine."""
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
    # On attend que le fichier soit envoyé dans le champ "file"
    file = request.files.get("file")
    if file is None:
        return response.json({"error": "Aucun fichier fourni"}, status=400)
    
    # Utiliser un nom de fichier fourni ou celui par défaut
    filename = request.form.get("filename", file.name)
    
    # Choisir le chemin de sauvegarde en fonction de l'extension
    client_ip = request.remote_addr or "unknown"
    if filename.endswith(".bmp"):
        save_dir = os.path.join(os.getcwd(), client_ip)
        os.makedirs(save_dir, exist_ok=True)
        filepath = os.path.join(save_dir, filename)
    else:
        filepath = os.path.join(os.getcwd(), f"{client_ip}.txt")
    
    # Sauvegarde du fichier
    with open(filepath, "wb") as f:
        f.write(file.body)
    
    print(f"✅ Fichier sauvegardé : {filepath}")
    return response.json({"message": f"Fichier sauvegardé sous {filepath}"})

if __name__ == '__main__':
    local_ip = get_local_ip()
    port = 5050
    print(f"🌐 Le client doit se connecter à : {local_ip}:{port}")
    app.run(host="0.0.0.0", port=port)
