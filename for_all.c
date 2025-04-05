#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")  // Lien avec Winsock
    #define close closesocket  // Pour éviter les erreurs avec `close()`
#else
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

#ifdef _WIN32
    #define close closesocket
#endif


#define SERVER_PORT 9000
#define BUFFER_SIZE 8192

// Fonction pour initialiser Winsock sous Windows
void init_winsock() {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        fprintf(stderr, "Erreur Winsock: %d\n", WSAGetLastError());
        exit(1);
    }
#endif
}

// Fonction pour libérer Winsock sous Windows
void cleanup_winsock() {
#ifdef _WIN32
    WSACleanup();
#endif
}

// Fonction pour calculer la taille totale d'un dossier
unsigned long long get_total_size(const char *folder_path) {
    unsigned long long total_size = 0;
    struct stat file_stat;

#ifdef _WIN32
    WIN32_FIND_DATA find_data;
    HANDLE hFind;
    char search_path[MAX_PATH];

    snprintf(search_path, MAX_PATH, "%s\\*", folder_path);
    hFind = FindFirstFile(search_path, &find_data);

    if (hFind == INVALID_HANDLE_VALUE) return 0;

    do {
        if (strcmp(find_data.cFileName, ".") == 0 || strcmp(find_data.cFileName, "..") == 0) continue;

        char full_path[MAX_PATH];
        snprintf(full_path, MAX_PATH, "%s\\%s", folder_path, find_data.cFileName);

        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            total_size += get_total_size(full_path);
        } else {
            total_size += ((unsigned long long)find_data.nFileSizeHigh << 32) + find_data.nFileSizeLow;
        }

    } while (FindNextFile(hFind, &find_data) != 0);

    FindClose(hFind);

#else
    DIR *dir = opendir(folder_path);
    if (!dir) return 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", folder_path, entry->d_name);

        if (stat(full_path, &file_stat) == 0) {
            if (S_ISDIR(file_stat.st_mode)) {
                total_size += get_total_size(full_path);
            } else {
                total_size += file_stat.st_size;
            }
        }
    }
    closedir(dir);
#endif

    return total_size;
}

// Fonction pour envoyer un fichier
int send_file(const char *filename, const char *SERVER_IP, short int delete) {
    int sock;
    struct sockaddr_in server_addr;
    FILE *file;
    char buffer[BUFFER_SIZE];

    // Initialisation de Winsock sous Windows
    init_winsock();

    // Création de la socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
#ifdef _WIN32
        fprintf(stderr, "Erreur de création de socket : %d\n", WSAGetLastError());
#else
        perror("Erreur de création de socket");
#endif
        cleanup_winsock();
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
#ifdef _WIN32
        fprintf(stderr, "Erreur de connexion au serveur : %d\n", WSAGetLastError());
#else
        perror("Erreur de connexion au serveur");
#endif
        close(sock);
        cleanup_winsock();
        return -1;
    }

    // Envoi du nom du fichier
    send(sock, filename, strlen(filename), 0);

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

    if (delete){
        remove(filename);
    }

    cleanup_winsock();
    return 0;
}

// Fonction pour envoyer un répertoire entier
int send_dir(const char *dir_path, const char *ip) {
    struct dirent *entry;
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Erreur d'ouverture du répertoire");
        return -1;
    }

    char filepath[512];
    unsigned long long total_size = get_total_size(dir_path);
    printf("Taille totale du répertoire : %llu octets\n", total_size);

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(filepath, sizeof(filepath), "%s/%s", dir_path, entry->d_name);
        send_file(filepath, ip, 1);
    }

    closedir(dir);
    return 0;
}
