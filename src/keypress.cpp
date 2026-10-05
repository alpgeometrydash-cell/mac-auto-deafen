#include "keypress.hpp"

#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <unistd.h>

namespace {
    // macOS virtual key codes for A-Z (US layout positions)
    constexpr CGKeyCode kLetterCodes[26] = {
        0x00, // A
        0x0B, // B
        0x08, // C
        0x02, // D
        0x0E, // E
        0x03, // F
        0x05, // G
        0x04, // H
        0x22, // I
        0x26, // J
        0x28, // K
        0x25, // L
        0x2E, // M
        0x2D, // N
        0x1F, // O
        0x23, // P
        0x0C, // Q
        0x0F, // R
        0x01, // S
        0x11, // T
        0x20, // U
        0x09, // V
        0x0D, // W
        0x07, // X
        0x10, // Y
        0x06, // Z
    };

    constexpr CGKeyCode kControl = 0x3B;
    constexpr CGKeyCode kOption  = 0x3A;
    constexpr CGKeyCode kShift   = 0x38;
    constexpr CGKeyCode kCommand = 0x37;

    void post(CGEventSourceRef src, CGKeyCode code, bool down, CGEventFlags flags) {
        CGEventRef ev = CGEventCreateKeyboardEvent(src, code, down);
        if (!ev) return;
        CGEventSetFlags(ev, flags);
        CGEventPost(kCGHIDEventTap, ev);
        CFRelease(ev);
        usleep(4000); // tiny gap so Discord registers each step
    }
}

void pressKeyCombo(char letter, bool ctrl, bool option, bool shift, bool command) {
    if (letter >= 'a' && letter <= 'z') letter = letter - 'a' + 'A';
    if (letter < 'A' || letter > 'Z') return;
    CGKeyCode key = kLetterCodes[letter - 'A'];

    CGEventSourceRef src = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);

    // Press modifiers one by one (flags build up), then the key, then release in reverse.
    CGEventFlags flags = 0;
    if (ctrl)    { flags |= kCGEventFlagMaskControl;   post(src, kControl, true, flags); }
    if (option)  { flags |= kCGEventFlagMaskAlternate; post(src, kOption,  true, flags); }
    if (shift)   { flags |= kCGEventFlagMaskShift;     post(src, kShift,   true, flags); }
    if (command) { flags |= kCGEventFlagMaskCommand;   post(src, kCommand, true, flags); }

    post(src, key, true, flags);
    post(src, key, false, flags);

    if (command) { flags &= ~kCGEventFlagMaskCommand;   post(src, kCommand, false, flags); }
    if (shift)   { flags &= ~kCGEventFlagMaskShift;     post(src, kShift,   false, flags); }
    if (option)  { flags &= ~kCGEventFlagMaskAlternate; post(src, kOption,  false, flags); }
    if (ctrl)    { flags &= ~kCGEventFlagMaskControl;   post(src, kControl, false, flags); }

    if (src) CFRelease(src);
}

bool hasAccessibilityPermission(bool prompt) {
    const void* keys[] = { kAXTrustedCheckOptionPrompt };
    const void* values[] = { prompt ? kCFBooleanTrue : kCFBooleanFalse };
    CFDictionaryRef opts = CFDictionaryCreate(
        kCFAllocatorDefault, keys, values, 1,
        &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks
    );
    bool trusted = AXIsProcessTrustedWithOptions(opts);
    CFRelease(opts);
    return trusted;
}

#else
// Non-mac builds: do nothing.
void pressKeyCombo(char, bool, bool, bool, bool) {}
bool hasAccessibilityPermission(bool) { return true; }
#endif
