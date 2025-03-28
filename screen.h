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

#include <windows.h>
#include <stdio.h>
#include <time.h>

void save_bitmap(HBITMAP hBitmap, const char *filename) {
    BITMAP bmp;
    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    PBITMAPINFO pbmi;
    WORD cClrBits;
    HANDLE hf;
    DWORD dwSizeofDIB;
    PBITMAPINFOHEADER pbih;
    PBYTE lpBits;
    DWORD dwWritten;

    // Récupérer les informations de l'image bitmap
    if (!GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bmp)) {
        printf("GetObject failed\n");
        return;
    }

    // Calculer la taille du fichier DIB
    cClrBits = (WORD)(bmp.bmBitsPixel * bmp.bmPlanes);
    if (cClrBits == 1) {
        cClrBits = 24;
    }

    // Configuration de l'en-tête du fichier
    bfh.bfType = 0x4D42; // "BM"
    bfh.bfReserved1 = 0;
    bfh.bfReserved2 = 0;
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    dwSizeofDIB = bmp.bmWidthBytes * bmp.bmHeight;
    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = bmp.bmWidth;
    bih.biHeight = bmp.bmHeight;
    bih.biPlanes = 1;
    bih.biBitCount = 24;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = dwSizeofDIB;
    bih.biXPelsPerMeter = 0;
    bih.biYPelsPerMeter = 0;
    bih.biClrUsed = 0;
    bih.biClrImportant = 0;

    // Créer le fichier
    hf = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, (DWORD)0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hf == INVALID_HANDLE_VALUE) {
        printf("CreateFile failed\n");
        return;
    }

    // Écrire l'en-tête du fichier BMP
    WriteFile(hf, (LPSTR)&bfh, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
    WriteFile(hf, (LPSTR)&bih, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);

    pbmi = (PBITMAPINFO)GlobalAlloc(GPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256);
    pbih = (PBITMAPINFOHEADER)pbmi;

    // Ecrire les données de l'image
    lpBits = (PBYTE)GlobalAlloc(GPTR, dwSizeofDIB);
    if (lpBits == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    GetDIBits(GetDC(0), hBitmap, 0, (UINT)bmp.bmHeight, lpBits, pbmi, DIB_RGB_COLORS);
    WriteFile(hf, lpBits, dwSizeofDIB, &dwWritten, NULL);

    // Libérer la mémoire et fermer le fichier
    GlobalFree(lpBits);
    GlobalFree(pbmi);
    CloseHandle(hf);
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
            HDC hdcScreen = GetDC(NULL);
            HDC hdcMemory = CreateCompatibleDC(hdcScreen);
            int screen_width = GetSystemMetrics(SM_CXSCREEN);
            int screen_height = GetSystemMetrics(SM_CYSCREEN);

            // Créer une image compatible avec la taille de l'écran
            HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, screen_width, screen_height);
            SelectObject(hdcMemory, hBitmap);

            // Copier l'image de l'écran dans le hBitmap
            BitBlt(hdcMemory, 0, 0, screen_width, screen_height, hdcScreen, 0, 0, SRCCOPY);

            // Créer un nom de fichier unique basé sur le compteur d'image
            char filename[1024];
            snprintf(filename, sizeof(filename), "%s\\screenshot_%d.bmp", output_directory, image_counter);

            // Sauvegarder l'image capturée dans le fichier
            save_bitmap(hBitmap, filename);
            printf("Image sauvegardée : %s\n", filename);

            // Libérer les ressources
            DeleteDC(hdcMemory);
            ReleaseDC(NULL, hdcScreen);
            DeleteObject(hBitmap);

            image_counter++;
            last_capture_time = current_time;
        }
    }
}

#endif
