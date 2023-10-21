#ifndef _DEBUGWINDOW_H_
#define _DEBUGWINDOW_H_

#include <ncurses.h>

void InitDebugWindow(WINDOW* win);
void SetDebugStatus(const char* newStatus);

#endif // _DEBUGWINDOW_H_