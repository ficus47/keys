#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")   // Lien avec Winsock
    #define close closesocket   // Pour éviter les erreurs avec `close()`
#else
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
#endif

#ifdef _WIN32
    #define close closesocket
#endif


#define SERVER_PORT 5050
#define BUFFER_SIZE 8192*8

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

int send_file(const char *filename, const char *SERVER_IP, short int delete) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("Erreur d'ouverture du fichier");
        return -1;
    }

    // Lire le contenu du fichier en mémoire
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    rewind(file);

    char *filedata = malloc(filesize);
    if (!filedata) {
        perror("Erreur d'allocation mémoire");
        fclose(file);
        return -1;
    }

    fread(filedata, 1, filesize, file);
    fclose(file);

    // Préparer la requête HTTP POST avec multipart/form-data
    char boundary[] = "----BOUNDARY1234567890";
    char header[1024];
    snprintf(header, sizeof(header),
            "--%s\r\n"
            "Content-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\n"
            "Content-Type: application/octet-stream\r\n\r\n",
            boundary, filename);

    char footer[64];
    snprintf(footer, sizeof(footer), "\r\n--%s--\r\n", boundary);

    int content_length = strlen(header) + filesize + strlen(footer);
    int total_length = content_length + 1024;

    char *request = malloc(total_length);
    if (!request) {
        perror("Erreur d'allocation mémoire (request)");
        free(filedata);
        return -1;
    }

    int offset = 0;
    offset += snprintf(request, total_length,
            "POST /upload HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "Content-Type: multipart/form-data; boundary=%s\r\n"
            "Content-Length: %d\r\n\r\n",
            SERVER_IP, SERVER_PORT, boundary, content_length);

    memcpy(request + offset, header, strlen(header));
    offset += strlen(header);
    memcpy(request + offset, filedata, filesize);
    offset += filesize;
    memcpy(request + offset, footer, strlen(footer));
    offset += strlen(footer);

    free(filedata);

    // Initialisation de Winsock sous Windows
    init_winsock();

    // Création de la socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Erreur de création de socket");
        free(request);
        cleanup_winsock();
        return -1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Adresse IP du serveur invalide");
        close(sock);
        free(request);
        cleanup_winsock();
        return -1;
    }

    // Connexion
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de connexion au serveur");
        close(sock);
        free(request);
        cleanup_winsock();
        return -1;
    }

    // Envoi de la requête HTTP
    ssize_t sent_bytes = send(sock, request, offset, 0);
    if (sent_bytes < 0) {
        perror("Erreur lors de l'envoi de la requête");
    } else if (sent_bytes != offset) {
        fprintf(stderr, "Erreur: Seuls %zd octets sur %d ont été envoyés.\n", sent_bytes, offset);
    }

    // Lire réponse (pas strictement nécessaire ici, mais utile pour le débuggage)
    char response[1024];
    ssize_t received_bytes = recv(sock, response, sizeof(response) - 1, 0);
    if (received_bytes > 0) {
        response[received_bytes] = '\0';
        printf("Réponse du serveur: %s\n", response);
    } else if (received_bytes < 0) {
        perror("Erreur lors de la réception de la réponse");
    } else {
        printf("Serveur a fermé la connexion.\n");
    }

    printf("✅ Fichier %s envoyé via HTTP avec succès !\n", filename);

    close(sock);
    free(request);

    if (delete) {
        if (remove(filename) == 0) {
            printf("🗑️ Fichier %s supprimé localement.\n", filename);
        } else {
            perror("Erreur lors de la suppression du fichier local");
        }
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
        printf("Envoi de : %s...\n", filepath);
        if (send_file(filepath, ip, 1) != 0) {
            fprintf(stderr, "Erreur lors de l'envoi de %s\n", filepath);
        }
    }

    closedir(dir);
    return 0;
}
