# server.py
import os
from http.server import BaseHTTPRequestHandler, HTTPServer

PORT = 8080
SEGMENTS_DIR = os.path.join(os.getcwd(), "file_segments")

class FileReceiverHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        # Lire le corps
        length = int(self.headers.get("Content-Length", 0))
        data = self.rfile.read(length)

        # Lire en-têtes custom
        filename = self.headers.get("X-Filename", "unknown")
        segment = int(self.headers.get("X-Segment-Number", "0"))
        total = int(self.headers.get("X-Total-Segments", "1"))

        print(f"▶️  Reçu segment {segment+1}/{total} pour '{filename}'")

        # Créer dossier segments
        os.makedirs(SEGMENTS_DIR, exist_ok=True)
        part_path = os.path.join(SEGMENTS_DIR, f"{filename}.part{segment}")
        with open(part_path, "wb") as f:
            f.write(data)

        # Si c’est le dernier attendu, assembler
        received = [
            f for f in os.listdir(SEGMENTS_DIR)
            if f.startswith(f"{filename}.part")
        ]
        if len(received) == total:
            print(f"✅ Tous les segments reçus pour '{filename}', assemblage…")
            output_path = os.path.join(os.getcwd(), filename)
            with open(output_path, "wb") as out:
                for i in range(total):
                    part_file = os.path.join(SEGMENTS_DIR, f"{filename}.part{i}")
                    with open(part_file, "rb") as pf:
                        out.write(pf.read())
                    os.remove(part_file)
            print(f"🎉 Fichier assemblé en '{output_path}'")

        # Répondre OK
        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"OK")

def run():
    print(f"🟢 Serveur HTTP en écoute sur :{PORT}")
    server = HTTPServer(("", PORT), FileReceiverHandler)
    server.serve_forever()

if __name__ == "__main__":
    run()
