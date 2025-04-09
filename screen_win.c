#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <direct.h> // Pour _mkdir

#include <stdlib.h>

void save_bitmap(HBITMAP hBitmap, const char *filename) {
    BITMAP bmp;
    if (!GetObject(hBitmap, sizeof(BITMAP), &bmp)) return;

    BITMAPFILEHEADER bfh = {0};
    BITMAPINFOHEADER bih = {0};

    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = bmp.bmWidth;
    bih.biHeight = bmp.bmHeight;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;

    DWORD dwSize = ((bmp.bmWidth * 3 + 3) & ~3) * bmp.bmHeight;
    bfh.bfType = 0x4D42;
    bfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwSize;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    BYTE *lpBits = (BYTE *)malloc(dwSize);
    if (!lpBits) return;

    HDC hdc = GetDC(NULL);
    GetDIBits(hdc, hBitmap, 0, bmp.bmHeight, lpBits, (BITMAPINFO *)&bih, DIB_RGB_COLORS);
    ReleaseDC(NULL, hdc);

    FILE *f = fopen(filename, "wb");
    if (!f) {
        free(lpBits);
        return;
    }

    fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, f);
    fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, f);
    fwrite(lpBits, dwSize, 1, f);
    fclose(f);
    free(lpBits);
}

void capture_screen_at_fps(int target_fps, const char *output_directory) {
    LARGE_INTEGER freq, last_time, current_time;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&last_time);

    double frame_interval = 1.0 / target_fps;
    int screen_width = GetSystemMetrics(SM_CXSCREEN);
    int screen_height = GetSystemMetrics(SM_CYSCREEN);

    while (1) {
        QueryPerformanceCounter(&current_time);
        double elapsed = (double)(current_time.QuadPart - last_time.QuadPart) / freq.QuadPart;

        if (elapsed >= frame_interval) {
            HDC hdcScreen = GetDC(NULL);
            HDC hdcMemory = CreateCompatibleDC(hdcScreen);
            HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screen_width, screen_height);
            SelectObject(hdcMemory, hBitmap);
            BitBlt(hdcMemory, 0, 0, screen_width, screen_height, hdcScreen, 0, 0, SRCCOPY);

            // Générer nom de fichier basé sur le timestamp Unix
            char filename[1024];
            time_t timestamp = time(NULL);
            snprintf(filename, sizeof(filename), "%s\\%lld.bmp", output_directory, (long long)timestamp);

            save_bitmap(hBitmap, filename);
            printf("Image sauvegardée : %s\n", filename);

            DeleteDC(hdcMemory);
            ReleaseDC(NULL, hdcScreen);
            DeleteObject(hBitmap);

            last_time = current_time;
        }

        // Pour ne pas saturer le CPU
        Sleep(1);
    }
}

int main(){
    makedir(.output_screen);
    
}