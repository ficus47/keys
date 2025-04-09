#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define SERVER_PORT 5050
#define BUFFER_SIZE 8192

// Fonction pour initialiser Winsock
void init_winsock() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "Erreur Winsock: %d\n", WSAGetLastError());
        exit(1);
    }
}

// Fonction pour nettoyer Winsock
void cleanup_winsock() {
    WSACleanup();
}

// Fonction pour envoyer un fichier
int send_file(const char *filename, const char *SERVER_IP) {
    int sock;
    struct sockaddr_in server_addr;
    FILE *file;
    char buffer[BUFFER_SIZE];

    // Initialisation de Winsock
    init_winsock();

    // Création de la socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur de création de socket");
        cleanup_winsock();
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);  // Utilisation de inet_addr pour IP

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de connexion au serveur");
        close(sock);
        cleanup_winsock();
        return -1;
    }

    // Envoi du nom du fichier
    char filename_with_newline[256];
    snprintf(filename_with_newline, sizeof(filename_with_newline), "%s\n", filename);
    send(sock, filename_with_newline, strlen(filename_with_newline), 0);

    // Ouverture du fichier à envoyer
    file = fopen(filename, "rb");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        close(sock);
        cleanup_winsock();
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

    cleanup_winsock();
    return 0;
}

int main() {
    const char *SERVER_IP = "10.0.11.87"; // Remplace par l'IP du serveur
    const char *filename = ".output_text/text.txt"; // Remplace par le nom du fichier à envoyer

    printf("📤 Envoi du fichier %s vers le serveur à %s:%d\n", filename, SERVER_IP, SERVER_PORT);

    // Envoi du fichier au serveur
    if (send_file(filename, SERVER_IP) != 0) {
        printf("❌ Échec de l'envoi du fichier\n");
        return 1;
    }

    return 0;
}
