#ifndef __KEYBOARD_H_
#define __KEYBOARD_H_

#include <stdbool.h>

#define KEYBOARD_WIDTH  200
#define KEYBOARD_HEIGHT 300

#define KEYBOARD_COLOR_PRESSED           UI_COLOR_HIGHLIGHT


typedef struct Keyboard {
	bool key_pressed[12];
} Keyboard;

void Keyboard_init();

#endif
