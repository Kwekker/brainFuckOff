#include <ncurses.h>

#include "color.h"


void InitColorPairs(void) {

    // Initialize all the colors.
    start_color();


    init_pair(BRACKET_PAIR,     COLOR_MAGENTA,  COLOR_BLACK);
    init_pair(MOVER_PAIR,       COLOR_YELLOW,   COLOR_BLACK);
    init_pair(INOUT_PAIR,       COLOR_CYAN,     COLOR_BLACK);
    init_pair(RUNNING_PAIR,     COLOR_WHITE,    COLOR_RED);
    init_pair(BREAKPOINT_PAIR,  COLOR_GREEN,    COLOR_BLACK);

    init_pair(MEM_CURSOR_PAIR,  COLOR_BLACK,    COLOR_CYAN);
    init_pair(MEM_USED_PAIR,    COLOR_WHITE,    COLOR_BLACK);
    init_pair(MEM_INDEX_PAIR,   COLOR_GREEN,    COLOR_BLACK);

    init_pair(DEBUG_PAIR,       COLOR_BLACK,    COLOR_CYAN);
    init_pair(TRUE_PAIR,        COLOR_BLACK,    COLOR_GREEN);
    init_pair(FALSE_PAIR,       COLOR_WHITE,    COLOR_RED);

    // Not every terminal supports custom colors apparently (cringe).
    if(can_change_color()) {
        init_color(COLOR_GREY, 400, 400, 400);

        init_pair(COMMENT_PAIR,  COLOR_GREY,  COLOR_BLACK);
        init_pair(CHANGER_PAIR,  COLOR_WHITE, COLOR_BLACK);
        init_pair(MEM_IDLE_PAIR, COLOR_GREY,  COLOR_BLACK);
    }
    else {
        init_pair(COMMENT_PAIR,  COLOR_WHITE, COLOR_BLACK);
        init_pair(CHANGER_PAIR,  COLOR_BLUE,  COLOR_BLACK);
        init_pair(MEM_IDLE_PAIR, COLOR_BLUE,  COLOR_BLACK);
    }
}