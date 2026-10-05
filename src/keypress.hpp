#pragma once

// Kept separate from Geode headers so macOS system headers don't clash with cocos2d names.

// Presses and releases `letter` (A-Z) while holding the chosen modifiers.
void pressKeyCombo(char letter, bool ctrl, bool option, bool shift, bool command);

// Returns true if macOS lets this app send keypresses.
// If `prompt` is true, macOS shows the "allow Accessibility" popup when it isn't allowed yet.
bool hasAccessibilityPermission(bool prompt);
