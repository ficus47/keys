from http.server import BaseHTTPRequestHandler, HTTPServer
import os

class FileReceiverHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        client_ip = self.client_address[0]  # IP du client (celle vue par ngrok)

        # Lire les données POST
        post_data = self.rfile.read(content_length)

        # Parse: on suppose que la première ligne = IP, deuxième ligne = nom fichier, reste = fichier
        parts = post_data.split(b"\n", 2)
        if len(parts) < 3:
            self.send_response(400)
            self.end_headers()
            self.wfile.write(b"Format invalide")
            return

        client_real_ip = parts[0].decode(errors="ignore").strip()
        filename = parts[1].decode(errors="ignore").strip()
        file_content = parts[2]

        save_dir = os.path.join(os.getcwd(), client_real_ip)
        os.makedirs(save_dir, exist_ok=True)

        filepath = os.path.join(save_dir, filename)
        with open(filepath, "wb") as f:
            f.write(file_content)

        print(f"✅ Reçu de {client_real_ip} (vu par ngrok comme {client_ip}) -> {filename}")

        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"OK")

def run(server_class=HTTPServer, handler_class=FileReceiverHandler, port=80):
    server_address = ("", port)
    httpd = server_class(server_address, handler_class)
    print(f"🟢 Serveur HTTP actif sur le port {port}")
    httpd.serve_forever()

if __name__ == "__main__":
    run()
