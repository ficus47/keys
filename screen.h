#ifdef __linux__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

void save_bitmap(XImage *image, const char *filename) {
    FILE *f = fopen(filename, "wb");
    if (f == NULL) {
        perror("fopen");
        return;
    }

    // BMP header
    unsigned char header[54] = {
        'B', 'M', 0, 0, 0, 0, 0, 0, 54, 0, 0, 0, 40, 0, 0, 0, 
        0, 0, 0, 0, 1, 0, 24, 0
    };

    // Image size
    unsigned int image_size = image->width * image->height * 3;
    unsigned int file_size = 54 + image_size;

    // Write the header with updated size
    header[2] = (file_size & 0xFF);
    header[3] = (file_size >> 8) & 0xFF;
    header[4] = (file_size >> 16) & 0xFF;
    header[5] = (file_size >> 24) & 0xFF;

    header[18] = (image->width & 0xFF);
    header[19] = (image->width >> 8) & 0xFF;
    header[20] = (image->width >> 16) & 0xFF;
    header[21] = (image->width >> 24) & 0xFF;

    header[22] = (image->height & 0xFF);
    header[23] = (image->height >> 8) & 0xFF;
    header[24] = (image->height >> 16) & 0xFF;
    header[25] = (image->height >> 24) & 0xFF;

    fwrite(header, sizeof(unsigned char), 54, f);

    // Write pixel data
    for (int y = image->height - 1; y >= 0; y--) {
        for (int x = 0; x < image->width; x++) {
            unsigned long pixel = XGetPixel(image, x, y);
            unsigned char red = (pixel & 0xFF0000) >> 16;
            unsigned char green = (pixel & 0x00FF00) >> 8;
            unsigned char blue = pixel & 0x0000FF;
            fwrite(&blue, sizeof(unsigned char), 1, f);
            fwrite(&green, sizeof(unsigned char), 1, f);
            fwrite(&red, sizeof(unsigned char), 1, f);
        }
    }

    fclose(f);
}

void capture_screen_at_fps(int target_fps, const char *output_directory) {
    Display *dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
        fprintf(stderr, "Cannot open X display\n");
        return;
    }

    Window root = DefaultRootWindow(dpy);
    XWindowAttributes gwa;
    XGetWindowAttributes(dpy, root, &gwa);

    int screen_width = gwa.width;
    int screen_height = gwa.height;

    clock_t last_capture_time = clock();
    int target_delay = CLOCKS_PER_SEC / target_fps;
    int image_counter = 0;

    while (1) {
        clock_t current_time = clock();
        int time_diff = current_time - last_capture_time;

        // Si le temps écoulé est plus grand que le délai cible, on capture
        if (time_diff >= target_delay) {
            XImage *image = XGetImage(dpy, root, 0, 0, screen_width, screen_height, AllPlanes, ZPixmap);
            if (image != NULL) {
                // Créer un nom de fichier unique basé sur le compteur d'image
                char filename[1024];
                snprintf(filename, sizeof(filename), "%s/screenshot_%d.bmp", output_directory, image_counter);
                save_bitmap(image, filename);
                printf("Image sauvegardée : %s\n", filename);
                XDestroyImage(image);
            }
            last_capture_time = current_time; // Réinitialiser l'heure de la dernière capture
            image_counter++;
        }
    }

    XCloseDisplay(dpy);
}

#endif

#ifdef __APPLE__

#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

void save_image(CGImageRef image, const char *filename) {
    CFURLRef url = CFURLCreateWithFileSystemPath(NULL, CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8), kCFURLPOSIXPathStyle, false);
    CGImageDestinationRef dest = CGImageDestinationCreateWithURL(url, kUTTypePNG, 1, NULL);
    CGImageDestinationAddImage(dest, image, NULL);
    CGImageDestinationFinalize(dest);
    CFRelease(url);
    CFRelease(dest);
}

void capture_screen_at_fps(int target_fps, const char *output_directory) {
    clock_t last_capture_time = clock();
    int target_delay = CLOCKS_PER_SEC / target_fps;
    int image_counter = 0;

    while (1) {
        clock_t current_time = clock();
        int time_diff = current_time - last_capture_time;

        // Si le temps écoulé est plus grand que le délai cible, on capture
        if (time_diff >= target_delay) {
            // Capture de l'écran
            CGImageRef image = CGDisplayCreateImage(kCGDirectMainDisplay);
            if (image != NULL) {
                // Crée un nom de fichier unique basé sur le compteur d'image
                char filename[1024];
                snprintf(filename, sizeof(filename), "%s/screenshot_%d.png", output_directory, image_counter);

                // Sauvegarde l'image dans le dossier spécifié
                save_image(image, filename);
                printf("Image sauvegardée : %s\n", filename);

                CFRelease(image);
                image_counter++; // Incrémenter le compteur d'images
            }

            last_capture_time = current_time; // Réinitialise l'heure de la dernière capture
        }
    }
}

#endif

#ifdef __WIN32__
#define COBJMACROS
#include <windows.h>
#include <wincodec.h>  // WIC (Windows Imaging Component)
#include <stdio.h>
#include <stdint.h>
#include <time.h>

#pragma comment(lib, "windowscodecs.lib")

void save_hbitmap_as_jpeg(HBITMAP hBitmap, const wchar_t *filename) {
    IWICImagingFactory *pFactory = NULL;
    IWICBitmapEncoder *pEncoder = NULL;
    IWICBitmapFrameEncode *pFrame = NULL;
    IWICStream *pStream = NULL;
    IWICBitmap *pBitmap = NULL;

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return;

    hr = CoCreateInstance(&CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER,
                          &IID_IWICImagingFactory, (LPVOID*)&pFactory);
    if (FAILED(hr)) goto cleanup;

    hr = IWICImagingFactory_CreateStream(pFactory, &pStream);
    if (FAILED(hr)) goto cleanup;

    hr = IWICStream_InitializeFromFilename(pStream, filename, GENERIC_WRITE);
    if (FAILED(hr)) goto cleanup;

    hr = IWICImagingFactory_CreateEncoder(pFactory, &GUID_ContainerFormatJpeg, NULL, &pEncoder);
    if (FAILED(hr)) goto cleanup;

    hr = IWICBitmapEncoder_Initialize(pEncoder, (IStream*)pStream, WICBitmapEncoderNoCache);
    if (FAILED(hr)) goto cleanup;

    hr = IWICBitmapEncoder_CreateNewFrame(pEncoder, &pFrame, NULL);
    if (FAILED(hr)) goto cleanup;

    hr = IWICBitmapFrameEncode_Initialize(pFrame, NULL);
    if (FAILED(hr)) goto cleanup;

    // Convert HBITMAP to IWICBitmap
    hr = IWICImagingFactory_CreateBitmapFromHBITMAP(pFactory, hBitmap, NULL, WICBitmapUseAlpha, &pBitmap);
    if (FAILED(hr)) goto cleanup;

    hr = IWICBitmapFrameEncode_WriteSource(pFrame, (IWICBitmapSource*)pBitmap, NULL);
    if (FAILED(hr)) goto cleanup;

    IWICBitmapFrameEncode_Commit(pFrame);
    IWICBitmapEncoder_Commit(pEncoder);

cleanup:
    if (pBitmap) IWICBitmap_Release(pBitmap);
    if (pFrame) IWICBitmapFrameEncode_Release(pFrame);
    if (pEncoder) IWICBitmapEncoder_Release(pEncoder);
    if (pStream) IWICStream_Release(pStream);
    if (pFactory) IWICImagingFactory_Release(pFactory);
    CoUninitialize();
}

void capture_screen_720p(const wchar_t *output_dir, int fps) {
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    int target_width = 1280;
    int target_height = 720;

    HDC hScreen = GetDC(NULL);
    HDC hdcFull = CreateCompatibleDC(hScreen);
    HDC hdcScaled = CreateCompatibleDC(hScreen);

    HBITMAP hbmpFull = CreateCompatibleBitmap(hScreen, width, height);
    HBITMAP hbmpScaled = CreateCompatibleBitmap(hScreen, target_width, target_height);

    SelectObject(hdcFull, hbmpFull);
    SelectObject(hdcScaled, hbmpScaled);

    while (1) {
        BitBlt(hdcFull, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);

        SetStretchBltMode(hdcScaled, HALFTONE);
        StretchBlt(hdcScaled, 0, 0, target_width, target_height, hdcFull, 0, 0, width, height, SRCCOPY);

        FILETIME ft;
        GetSystemTimeAsFileTime(&ft);
        ULONGLONG time = ((ULONGLONG)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
        ULONGLONG ms = (time - 116444736000000000ULL) / 10000;

        wchar_t filename[512];
        swprintf(filename, 512, L"%s\\%llu.jpg", output_dir, ms);
        save_hbitmap_as_jpeg(hbmpScaled, filename);

        wprintf(L"[+] Sauvé : %s\n", filename);
        Sleep(1000 / fps);
    }

    DeleteObject(hbmpFull);
    DeleteObject(hbmpScaled);
    DeleteDC(hdcFull);
    DeleteDC(hdcScaled);
    ReleaseDC(NULL, hScreen);
}


#endif
