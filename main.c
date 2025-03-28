#include "keylogger.h"
#include "screen.h"

const char *output_dir = ".output_screen"

#ifdef __linux__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    pid_t pid;

    pid = fork();  // Créer un nouveau processus
    if (pid < 0) {
        // Échec de fork
        perror("Erreur de fork");
        exit(1);
    }

    if (pid == 0) {
        // Processus enfant
        char *args[] = {"/chemin/vers/ton/programme", "1"};  // Remplace par ton programme et arguments
        execvp(args[0], args);  // Exécute le programme avec les arguments
        perror("Erreur de execvp");  // Si execvp échoue
        exit(1);
    } else {
        // Processus parent attend que l'enfant se termine
        wait(NULL);
    }

    return 0;
};


#endif

#ifdef __APPLE__


#endif

#ifdef __WIN32__

#include <windows.h>
#include <stdio.h>

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
    char path[MAX_PATH];

    // Récupérer le chemin de l'exécutable
    if (GetModuleFileName(NULL, path, sizeof(path)) == 0) {
        printf("Erreur lors de la récupération du chemin de l'exécutable\n");
        return 1;
    }

    if (argc < 0){

    }
    else if (argc < 0) {
        
    }


}

#endif