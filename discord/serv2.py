import os
from http.server import BaseHTTPRequestHandler, HTTPServer

# Port dédié pour l'assemblage des fichiers .txt
PORT = 8081  
SEGMENTS_DIR = os.path.join(os.getcwd(), "file_segments_dedicated")
TEXT_DIR = os.getcwd()

class TextFileReceiverHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        data = self.rfile.read(length)

        filename = self.headers.get("X-Filename", "unknown")
        file_type = self.headers.get("X-File-Type", "unknown")
        segment = int(self.headers.get("X-Segment-Number", "0"))
        total = int(self.headers.get("X-Total-Segments", "1"))

        if file_type != "output_text":
            self.send_response(400)
            self.end_headers()
            return

        # 1) Enregistrer le segment
        os.makedirs(SEGMENTS_DIR, exist_ok=True)
        part_path = os.path.join(SEGMENTS_DIR, f"{filename}.part{segment}")
        with open(part_path, "wb") as f:
            f.write(data)
        print(f"▶️  Segment {segment + 1}/{total} reçu pour '{filename}'")

        # 2) Si tous reçus, assembler
        parts = [f for f in os.listdir(SEGMENTS_DIR) if f.startswith(f"{filename}.part")]
        if len(parts) == total:
            print(f"🧩 Tous les segments reçus pour '{filename}', assemblage…")
            self.assemble_file(filename)

        # 3) Réponse
        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"OK")

    def assemble_file(self, filename):
        out_path = os.path.join(TEXT_DIR, filename)

        # Assembler le fichier à partir des segments
        with open(out_path, "wb") as out:
            for i in range(int(self.headers["X-Total-Segments"])):
                part_file = os.path.join(SEGMENTS_DIR, f"{filename}.part{i}")
                with open(part_file, "rb") as pf:
                    out.write(pf.read())
                os.remove(part_file)  # Supprimer le segment après assemblage

        print(f"✅ Fichier assemblé et sauvegardé en '{out_path}'")

def run():
    server = HTTPServer(("", PORT), TextFileReceiverHandler)
    print(f"🟢 Serveur dédié HTTP écoute sur le port {PORT}")
    server.serve_forever()

if __name__ == "__main__":
    run()
