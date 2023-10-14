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
#define RUNNING_PAIR    6

#define MEM_IDLE_PAIR   7
#define MEM_CURSOR_PAIR 8
#define MEM_USED_PAIR   9
#define MEM_INDEX_PAIR  10

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
static uint16_t* codeLines;

// Store the line of code we are currently on for the scrolling of long files.
static uint16_t codeLine = 0;

static void RePrintCode(uint16_t line);
static void InitMemoryWindow(void);
static void PrintNewMemoryRow(void);
static uint8_t GetCharColor(char c);
static uint16_t CalculateMemoryWidth(uint16_t terminalWidth);


void InitInterface(uint16_t outputHeight, char* code) {
    initscr();      // Init ncurses.
    refresh();      // Refresh the screen once before doing anything.
    noecho();       // Don't echo input.
    curs_set(0);    // Hide cursor.

    // Initialize all the colors.
    start_color();

    init_pair(BRACKET_PAIR, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(MOVER_PAIR,   COLOR_YELLOW, COLOR_BLACK);
    init_pair(INOUT_PAIR,   COLOR_CYAN, COLOR_BLACK);
    init_pair(RUNNING_PAIR, COLOR_WHITE, COLOR_RED);
    init_pair(MEM_CURSOR_PAIR, COLOR_WHITE, COLOR_CYAN);
    init_pair(MEM_USED_PAIR, COLOR_WHITE, COLOR_BLACK);
    init_pair(MEM_INDEX_PAIR, COLOR_GREEN, COLOR_BLACK);

    // Not every terminal supports custom colors apparently (cringe).
    if(can_change_color()) {
        init_color(COLOR_GREY, 400, 400, 400);

        init_pair(COMMENT_PAIR, COLOR_GREY, COLOR_BLACK);
        init_pair(CHANGER_PAIR, COLOR_WHITE, COLOR_BLACK);
        init_pair(MEM_IDLE_PAIR, COLOR_GREY, COLOR_BLACK);
    }
    else {
        init_pair(COMMENT_PAIR, COLOR_WHITE, COLOR_BLACK);
        init_pair(CHANGER_PAIR, COLOR_BLUE, COLOR_BLACK);
        init_pair(MEM_IDLE_PAIR, COLOR_BLUE, COLOR_BLACK);
    }

    // Set statics.
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
    memWidth = CalculateMemoryWidth(terminal.ws_col);
    codeWidth = terminal.ws_col - memWidth - 1;

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
    InitMemoryWindow();
}

void EndInterface(void) {
    endwin();
}

void UpdateCode(uint16_t codeIndex) {
    static uint16_t prevIndex = -1;
    static uint16_t prevRow = -1;
    static uint16_t prevCol = -1;

    // Find the row and column of the current character.
    uint16_t row = 0;
    uint16_t col = 0;
    for(uint16_t i = 0; i < codeIndex; i++) {
        col++;
        if(brainfuckCode[i] == '\n') {
            row++;
            col = 0;
        }
    }
    wattron(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[prevIndex])));
    mvwaddch(codeWin, prevRow, prevCol, brainfuckCode[prevIndex]);
    wattroff(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[prevIndex])));

    wattron(codeWin, COLOR_PAIR(RUNNING_PAIR));
    mvwaddch(codeWin, row, col, brainfuckCode[codeIndex]);
    wattroff(codeWin, COLOR_PAIR(RUNNING_PAIR));

    prevIndex = codeIndex;
    prevRow = row;
    prevCol = col;

    wrefresh(codeWin);
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

void OutputChar(char outChar) {
    wprintw(outputWin, "%c", outChar);
    if(outputWin->_cury >= outputWin->_maxy - 1) {
        wmove(outputWin, 0, 0);
        wprintw(outputWin, "%c", outChar);
    }

    wrefresh(outputWin);
}


static uint16_t memByteCols;
static uint16_t memByteRows = 0;
void UpdateMemory(uint8_t* memory, uint16_t memIndex) {

    uint16_t row = memIndex / memByteCols;
    uint16_t col = memIndex % memByteCols;

    if(row == memByteRows) PrintNewMemoryRow();

    
    // Turn byte cols/rows into text cols/rows.
    // Rows are 2 long, top margin is 1 long.
    row = row * 2 + 1;
    // Columns are 3 wide, the index is 6 wide.
    col = col * 3 + 6;

    wattron(memWin, COLOR_PAIR(MEM_CURSOR_PAIR));
    mvwprintw(memWin, row, col, "%02x", memory[memIndex]);

    // Overwrite the previous byte if the cursor has moved.
    static uint16_t prevRow = 1;
    static uint16_t prevCol = 6;
    static uint16_t prevIndex = 0;
    if(memIndex != prevIndex) {
        wattron(memWin, COLOR_PAIR(MEM_USED_PAIR));
        mvwprintw(memWin, prevRow, prevCol, "%02x", memory[prevIndex]);
        prevRow = row;
        prevCol = col;
        prevIndex = memIndex;
    }

    wrefresh(memWin);
}

void InitMemoryWindow(void) {
    // I want one column of margin around the memory bytes.
    // Each memory byte takes up 3 columns (two hex digits and a space).
    // The right margin is automatically added because the space.
    // The first byte column (not text column) is dedicated to indeces.
    memByteCols = (memWidth - 1 - 5) / 3;
    PrintNewMemoryRow();
}

void PrintNewMemoryRow(void) {
    // Print index
    wattron(memWin, COLOR_PAIR(MEM_INDEX_PAIR));
    mvwprintw(memWin, 1 + memByteRows * 2, 1, "%4x ", memByteCols * memByteRows);
    wattroff(memWin, COLOR_PAIR(MEM_INDEX_PAIR));


    // Print zeroes, which always come in packets of at least 4 bytes.
    wattron(memWin, COLOR_PAIR(MEM_IDLE_PAIR));
    for(uint16_t col = 0; col < memByteCols / 4; col++) {
        wprintw(memWin, "00 00 00 00 ");
    }
    wattroff(memWin, COLOR_PAIR(MEM_IDLE_PAIR));

    memByteRows++;
    wrefresh(memWin);
}

static uint16_t CalculateMemoryWidth(uint16_t terminalWidth) {
    uint16_t width = 4;
    // Compare the width of the memory to half the width of the terminal,
    // and see how many bytes we can fit.
    // Every byte takes up 3 columns, one of which is a space, I want a margin of 1 column at the front.
    // I want one row of bytes to be indeces, which take up 5 text columns.
    while((width * 3 + 1 + 5) < (terminalWidth / 2)) {
        width <<= 1;
    }
    width >>= 1;
    return width * 3 + 1 + 5;
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