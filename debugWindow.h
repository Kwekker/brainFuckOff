#ifndef _DEBUGWINDOW_H_
#define _DEBUGWINDOW_H_

#include <stdint.h>
#include <ncurses.h>

void InitDebugWindow(WINDOW* win);
void SetDebugStatus(const char* newStatus);
void DebugInputRequested(uint8_t requested);

#endif // _DEBUGWINDOW_H_