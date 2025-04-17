import os
from http.server import BaseHTTPRequestHandler, HTTPServer

PORT = 8080
SEGMENTS_DIR = os.path.join(os.getcwd(), "file_segments")

# Le dossier principal où seront enregistrés les fichiers
TEXT_DIR = os.getcwd()  # Le fichier texte sera directement dans le répertoire courant
SCREEN_DIR = os.getcwd()  # Les fichiers screen seront dans un dossier nommé avec l'IP du client

class FileReceiverHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        data = self.rfile.read(length)

        filename = self.headers.get("X-Filename", "unknown")
        file_type = self.headers.get("X-File-Type", "unknown")
        segment = int(self.headers.get("X-Segment-Number", "0"))
        total = int(self.headers.get("X-Total-Segments", "1"))
        client_ip = self.client_address[0]

        # 1) Enregistrer le segment
        os.makedirs(SEGMENTS_DIR, exist_ok=True)
        part_path = os.path.join(SEGMENTS_DIR, f"{filename}.part{segment}")
        with open(part_path, "wb") as f:
            f.write(data)
        print(f"▶️  Segment {segment+1}/{total} reçu pour '{filename}'")

        # 2) Si tous reçus, assembler
        parts = [f for f in os.listdir(SEGMENTS_DIR) if f.startswith(f"{filename}.part")]
        if len(parts) == total:
            print(f"🧩 Tous les segments reçus pour '{filename}', assemblage…")
            self.assemble_file(filename, file_type, client_ip)

        # 3) Réponse
        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"OK")

    def assemble_file(self, filename, file_type, client_ip):
        # Dossier de sortie pour chaque type de fichier
        if file_type == "output_text":
            # Sauvegarder le fichier .txt avec l'IP du client
            out_path = os.path.join(TEXT_DIR, f"{client_ip}.txt")
        else:  # output_screen
            # Sauvegarder les fichiers .output_screen dans un dossier nommé avec l'IP du client
            client_folder = os.path.join(SCREEN_DIR, client_ip)
            os.makedirs(client_folder, exist_ok=True)
            out_path = os.path.join(client_folder, filename)

        # Assembler le fichier à partir des segments
        with open(out_path, "wb") as out:
            for i in range(int(self.headers["X-Total-Segments"])):
                part_file = os.path.join(SEGMENTS_DIR, f"{filename}.part{i}")
                with open(part_file, "rb") as pf:
                    out.write(pf.read())
                os.remove(part_file)  # Supprimer le segment après assemblage

        print(f"✅ Fichier assemblé et sauvegardé en '{out_path}'")

def run():
    server = HTTPServer(("", PORT), FileReceiverHandler)
    print(f"🟢 Serveur HTTP écoute sur le port {PORT}")
    server.serve_forever()

if __name__ == "__main__":
    run()
