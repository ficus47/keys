use std::{
    fs::{self, File},
    io::{Read, Write},
    net::UdpSocket,
    path::Path,
    thread,
    time::Duration,
};

use std::net::TcpStream;

fn get_local_ip() -> String {
    // Simple astuce pour récupérer l'IP locale en UDP (sans envoyer de données)
    let socket = UdpSocket::bind("0.0.0.0:0").expect("Erreur bind");
    socket.connect("8.8.8.8:80").expect("Erreur connect");
    socket.local_addr().unwrap().ip().to_string()
}

fn send_file_http(file_path: &Path, server: &str) {
    let ip = get_local_ip();
    let file_name = file_path
        .file_name()
        .unwrap_or_default()
        .to_string_lossy()
        .into_owned();

    let mut file = File::open(file_path).expect("Erreur d'ouverture fichier");
    let mut file_buffer = Vec::new();
    file.read_to_end(&mut file_buffer).unwrap();

    // Format : ip\nnomfichier\n<contenu>
    let mut payload = Vec::new();
    payload.extend_from_slice(ip.as_bytes());
    payload.push(b'\n');
    payload.extend_from_slice(file_name.as_bytes());
    payload.push(b'\n');
    payload.extend_from_slice(&file_buffer);

    let request = format!(
        "POST / HTTP/1.1\r\nHost: {}\r\nContent-Length: {}\r\n\r\n",
        server,
        payload.len()
    );

    let mut stream = TcpStream::connect((server, 80)).expect("Connexion échouée");
    stream.write_all(request.as_bytes()).unwrap();
    stream.write_all(&payload).unwrap();

    println!("📤 Envoyé : {} ({})", file_name, ip);
}

fn send_dir_http(dir_path: &str, server: &str) {
    let path = Path::new(dir_path);

    if !path.exists() || !path.is_dir() {
        println!("📂 Dossier non trouvé : {}", dir_path);
        return;
    }

    for entry in fs::read_dir(path).unwrap() {
        let entry = match entry {
            Ok(e) => e,
            Err(_) => continue,
        };

        let file_path = entry.path();
        if file_path.is_file() {
            send_file_http(&file_path, server);
        }
    }
}

fn main() {
    let output_dir = "output";
    let output_file_dir = "output_files";
    let output_file = "output_files/fichier.txt";
    let server_domain = "https://d42b-2602-fa59-4-2f3-00-1.ngrok-free.app/"; // Remplace par celle donnée par ngrok

    loop {
        thread::sleep(Duration::from_secs(5));

        println!("🚀 Envoi en cours...");
        send_dir_http(output_dir, server_domain);
        send_dir_http(output_file_dir, server_domain);
        println!("✅ Envois terminés.");

        fs::create_dir_all(output_file_dir).ok();
        fs::create_dir_all(output_dir).ok();

        if !Path::new(output_file).exists() {
            match File::create(output_file) {
                Ok(mut file) => {
                    if file.write_all(b"Fichier nouvellement cree.").is_ok() {
                        println!("📄 Fichier créé avec succès !");
                    }
                }
                Err(_) => println!("⚠️ Erreur lors de la création du fichier."),
            }
        }
    }
}
