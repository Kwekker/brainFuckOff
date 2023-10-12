#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "interface.h"


#define DEBUG_WINDOW_HEIGHT 8
#define INPUT_WINDOW_HEIGHT 2

// Brainfuck character pairs
#define BRACKET_PAIR    1
#define MOVER_PAIR      2
#define CHANGER_PAIR    3
#define INOUT_PAIR      4
#define COMMENT_PAIR    5

#define COLOR_GREY      8

static uint16_t memWidth;
static uint16_t memHeight;
 
static uint16_t codeWidth;
static uint16_t codeHeight;
 
static WINDOW *codeWin;
static WINDOW *outputWin;
static WINDOW *memWin;
static WINDOW *debugWin;
static WINDOW *inputWin;
 
static char* brainfuckCode;

// Store the line of code we are currently on for the scrolling of long files.
static uint16_t codeLine = 0;

static void RePrintCode(uint16_t line);
static uint8_t GetCharColor(char c);


void InitInterface(uint16_t memoryColumns, uint16_t outputHeight, char* code) {
    initscr();
    refresh();
    start_color();

    init_pair(BRACKET_PAIR, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(MOVER_PAIR,   COLOR_YELLOW, COLOR_BLACK);
    init_pair(INOUT_PAIR,   COLOR_CYAN, COLOR_BLACK);

    if(can_change_color()) {
        init_color(COLOR_GREY, 400, 400, 400);
        init_pair(COMMENT_PAIR, COLOR_GREY, COLOR_BLACK);
        init_pair(CHANGER_PAIR, COLOR_WHITE, COLOR_BLACK);
    }
    else {
        init_pair(COMMENT_PAIR, COLOR_WHITE, COLOR_BLACK);
        init_pair(CHANGER_PAIR, COLOR_BLUE, COLOR_BLACK);
    }

    brainfuckCode = code;

    // Get terminal size.
    // Resizing is not supported because I prefer not having headaches.
    struct winsize terminal;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &terminal);

    // Define the heights of the windows, which are defined as the height of the terminal
    // minus the height of the other windows, minus the height of the seperation lines.
    codeHeight = terminal.ws_row - outputHeight - 1;
    memHeight = terminal.ws_row - DEBUG_WINDOW_HEIGHT - INPUT_WINDOW_HEIGHT - 2;

    // Define the width of the windows.
    // Make sure to leave room for a line between the left and right windows.
    codeWidth = terminal.ws_col / 2 - !(terminal.ws_col % 2);
    memWidth = terminal.ws_col / 2;

    // Define the windows.
    codeWin   = newwin(codeHeight, codeWidth, 0, 0);
    outputWin = newwin(terminal.ws_row - codeHeight - 1, codeWidth, codeHeight + 1, 0);
    memWin    = newwin(memHeight, memWidth, 0, codeWidth + 1);
    inputWin  = newwin(INPUT_WINDOW_HEIGHT, memWidth, memHeight + 1, codeWidth + 1);
    debugWin  = newwin(DEBUG_WINDOW_HEIGHT, memWidth, memHeight + INPUT_WINDOW_HEIGHT + 2, codeWidth + 1);

    // Print vertical window border.
    for(uint16_t i = 0; i < terminal.ws_row; i++) mvaddch(i, codeWidth, ACS_VLINE);

    // Print horizontal window borders.
    for(uint16_t i = 0; i < codeWidth; i++) mvaddch(codeHeight, i, ACS_HLINE);
    for(uint16_t i = codeWidth + 1; i < terminal.ws_col; i++) mvaddch(memHeight, i, ACS_HLINE);
    for(uint16_t i = codeWidth + 1; i < terminal.ws_col; i++) mvaddch(memHeight + INPUT_WINDOW_HEIGHT + 1, i, ACS_HLINE);

    // Print cool connection characters (├ and ┤).
    mvaddch(codeHeight, codeWidth, ACS_RTEE);
    mvaddch(memHeight, codeWidth, ACS_LTEE);
    mvaddch(memHeight + INPUT_WINDOW_HEIGHT + 1, codeWidth, ACS_LTEE);
    refresh();

    // Refresh all of the windows at the beginning.
    wrefresh(codeWin);
    wrefresh(memWin);
    wrefresh(outputWin);
    wrefresh(debugWin);
    wrefresh(inputWin);

    // Print code into the code window.
    RePrintCode(0);

    getch();
}

void EndInterface(void) {
    endwin();
}

void UpdateCode(uint16_t index) {

}

void RePrintCode(uint16_t line) {
    char* c = brainfuckCode;

    while(*c) {
        wattron(codeWin, COLOR_PAIR(GetCharColor(*c)));
        waddch(codeWin, *c);
        wattroff(codeWin, COLOR_PAIR(GetCharColor(*c)));

        c++;
    }

    wrefresh(codeWin);
}

void OutputPutChar(char outChar) {
    waddch(outputWin, outChar);
}

uint8_t GetCharColor(char c) {
    switch(c) {
        case '[':
        case ']':
            return BRACKET_PAIR;

        case '>':
        case '<':
            return MOVER_PAIR;

        case '+':
        case '-':
            return CHANGER_PAIR;

        case '.':
        case ',':
            return INOUT_PAIR;

        default:
            return COMMENT_PAIR;
    }
}