import socket

# 🔧 IP de ton serveur ici (modifie si nécessaire)
SERVER_IP = "10.0.11.87"  # ← remplace par l'IP de ton serveur
SERVER_PORT = 5050

try:
    print(f"📡 Tentative de connexion à {SERVER_IP}:{SERVER_PORT} ...")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((SERVER_IP, SERVER_PORT))
        print("✅ Connexion réussie !")
        s.sendall(b"Hello from client")
        print("📤 Message envoyé.")
except Exception as e:
    print(f"❌ Échec de connexion : {e}")
