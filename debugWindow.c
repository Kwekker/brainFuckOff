#include "debugWindow.h"
#include "color.h"

#define STATUS_COL 0
#define STATUS_LENGTH 20

WINDOW* debugWin;

void InitDebugWindow(WINDOW* win) {
    debugWin = win;

    wbkgd(debugWin, COLOR_PAIR(DEBUG_PAIR));
    wclear(debugWin);

    wrefresh(debugWin);
}

void SetDebugStatus(const char* newStatus) {
    wmove(debugWin, 0, STATUS_COL);
    for(uint8_t i = 0; i < STATUS_LENGTH; i++) waddch(debugWin, ' ');
    mvwprintw(debugWin, 0, STATUS_COL, "Status: %s", newStatus);
    wrefresh(debugWin);
}
