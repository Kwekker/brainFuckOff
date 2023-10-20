#ifndef _CODEWINDOW_H_
#define _CODEWINDOW_H_

#include <stdint.h>
#include <ncurses.h>

// The amount of extra lines to scroll down
// when the code pointer is above the current code screen.
#define CODE_TOP_MARGIN 2

// The amount of extra lines to scroll up
// when the code pointer is under the current code screen.
#define CODE_BOTTOM_MARGIN 4

void InitCodeWindow(WINDOW* window, char* code);
void UpdateCode(uint16_t codeIndex);

#endif // _CODEWINDOW_H_