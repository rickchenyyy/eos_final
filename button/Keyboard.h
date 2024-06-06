#pragma once

struct Keyboard;
typedef struct Keyboard Keyboard;
typedef struct Keyboard *KeyboardPtr;

KeyboardPtr createKeyboard();
void destroyKeyboard(KeyboardPtr keyboard);

// read-only file descriptor
int keyboardGetFileDescriptor(KeyboardPtr keyboard);
