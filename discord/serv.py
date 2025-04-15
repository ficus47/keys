import socket
import os

HOST = "0.0.0.0"
PORT = 8888

def start_server():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind((HOST, PORT))
    server_socket.listen(5)
    print(f"🟢 Serveur TCP en écoute sur {HOST}:{PORT}")

    while True:
        client_socket, client_address = server_socket.accept()
        print(f"📥 Connexion de {client_address[0]}")

        try:
            data = b""
            while True:
                chunk = client_socket.recv(4096)
                if not chunk:
                    break
                data += chunk

            if b"\n" not in data:
                print("⛔ Données mal formées, nom de fichier manquant.")
                continue

            # Séparer nom de fichier et contenu
            filename_raw, file_content = data.split(b"\n", 1)
            filename = filename_raw.decode(errors="ignore").strip()

            # Créer dossier selon IP
            save_dir = os.path.join(os.getcwd(), client_address[0])
            os.makedirs(save_dir, exist_ok=True)

            filepath = os.path.join(save_dir, filename)
            with open(filepath, "wb") as f:
                f.write(file_content)

            print(f"✅ Fichier reçu : {filepath}")
        except Exception as e:
            print(f"❌ Erreur pendant la réception : {e}")
        finally:
            client_socket.close()

if __name__ == "__main__":
    start_server()
