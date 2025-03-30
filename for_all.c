#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <dirent.h>

//#define SERVER_IP "192.168.1.100"  // IP du serveur
#define SERVER_PORT 9000
#define BUFFER_SIZE 4096

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

int send_dir(const char *dir_path, const char *ip){
    struct dirent *entry;
    DIR *dir = opendir(".");

    while ((entry = readdir(dir)) != NULL){
        
    }
}