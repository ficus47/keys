import socket
import threading
import os

socket.setdefaulttimeout(5)

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
our_ip = socket.gethostbyname(socket.gethostname())
s.bind((our_ip, 0))
s.listen(10000)
print("Server started, waiting for connections...")
print(f"Listening on {our_ip}:{s.getsockname()[1]}")

def handle_client(conn, addr):
    print(f"🔗 Connexion de {addr[0]}:{addr[1]}")
    try:
        # Lire le nom du fichier
        filename_bytes = b""
        while True:
            byte = conn.recv(1)
            if byte == b"" or byte == b"\n":
                break
            filename_bytes += byte

        filename = filename_bytes.decode(errors="ignore").strip()
        print(f"📄 Nom du fichier reçu : {filename}")

        if filename.startwith(".bmp"):

            # Sauvegarde dans un dossier
            os.makedirs(str(addr[0]), exist_ok=True)
            filepath = os.path.join(str(addr[0]), filename)

            with open(filepath, "wb") as f:
                while True:
                    data = conn.recv(BUFFER_SIZE)
                    if not data:
                        break
                    f.write(data)

            print(f"✅ Fichier sauvegardé : {filepath}")
        else:

            with open(str(addr[0])+".txt", "wb") as f:
                while True:
                    data = conn.recv(BUFFER_SIZE)
                    if not data:
                        break
                    f.write(data)

            print(f"✅ Fichier sauvegardé : {filepath}")

    except Exception as e:
        print(f"❌ Erreur avec {addr}: {e}")
    finally:
        conn.close()
        print(f"❎ Déconnexion de {addr}")

def receive_file_server():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    our_ip = socket.gethostbyname(socket.gethostname())
    s.bind((our_ip, 0))
    server.listen()

    print("Server started, waiting for connections...")
    print(f"📡 Adresse IP : {our_ip}")
    print(f"🔌 Port : {server.getsockname()[1]}")

    BUFFER_SIZE = 1024 * 1024  # 1 Mo
    
    while True:
        conn, addr = server.accept()
        client_thread = threading.Thread(target=handle_client, args=(conn, addr))
        client_thread.start()

receive_file_server()