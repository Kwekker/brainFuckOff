#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <sys/ioctl.h>

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

static void InitMemoryWindow(void);
static void PrintNewMemoryRow(void);
static uint16_t CalculateMemoryWidth(uint16_t terminalWidth);

static void InitCodeWindow(void);
static uint8_t GetCharColor(char c);


void InitInterface(uint16_t outputHeight, char* code) {
    initscr();      // Init ncurses.
    refresh();      // Refresh the screen once before doing anything.
    noecho();       // Don't echo input.
    curs_set(0);    // Hide cursor.

    brainfuckCode = code;

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

    // The output window needs to be able to scroll.
    scrollok(outputWin, 1);

    // Print code into the code window.
    InitCodeWindow();
    InitMemoryWindow();

}

void EndInterface(void) {
    endwin();
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


// ######## The code window ########
// This should probably go into a seperate file, but that is a lot of work, and git blame stops working.

// codeLines is an array that stores the first index of every line in the code. Lines include newlines, but also width overflows.
static uint16_t* codeLines;
static uint16_t codeLinesAmount;
static uint16_t codeLinesArraySize;
static uint16_t currentCodeLine = 0;
static void StoreCodeLine(uint16_t index);


// Print the first window of code.
void InitCodeWindow(void) {
    // Init the codeLines array with like idk 2 times the window height.
    codeLinesArraySize = 2 * codeHeight;
    codeLines = (uint16_t*) malloc(codeLinesArraySize * sizeof(uint16_t));

    // Print the code up to the end of the window.
    char* codePtr = brainfuckCode;
    wmove(codeWin, 0, 0);

    while(*codePtr) {

        // Store the line index.
        if(getcurx(codeWin) == 0) StoreCodeLine(codePtr - brainfuckCode);

        wattron(codeWin, COLOR_PAIR(GetCharColor(*codePtr)));
        int failed = waddch(codeWin, *codePtr);
        wattroff(codeWin, COLOR_PAIR(GetCharColor(*codePtr)));

        if(failed == ERR) break;

        codePtr++;
    }

    // Store the last (out of window) codeLine.
    StoreCodeLine(codePtr - brainfuckCode + 1);

    wrefresh(outputWin);
    wrefresh(codeWin);
}


void PrintNewLinesToIndex(uint16_t index) {


    // Start at the first character of the last line.
    // This is always the line after the lastly printed line.
    uint16_t codeIndex = codeLines[codeLinesAmount - 1];
    wprintw(outputWin, "%d %d\n", index, codeIndex);
    wrefresh(outputWin);

    
    // Print lines.
    while(codeIndex <= index) {
        // Scroll down one line.
        scrollok(codeWin, 1);
        scroll(codeWin);
        scrollok(codeWin, 0);
        wmove(codeWin, getmaxy(codeWin) - 1, 0);

        // Print one line.
        int endOfLine = 0;
        while(!endOfLine) {
            wattron(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[codeIndex])));
            endOfLine = waddch(codeWin, brainfuckCode[codeIndex]);
            wattroff(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[codeIndex])));
            codeIndex++;
        }

        // Store the next line's index.
        currentCodeLine++;
        StoreCodeLine(codeIndex);
    }
    
    wrefresh(codeWin);
    wrefresh(outputWin);
}


void StoreCodeLine(uint16_t index) {
    codeLines[codeLinesAmount++] = index;

    // Reallocate codeLines if it too big.
    if(codeLinesAmount + 1 == codeLinesArraySize) {
        codeLinesArraySize += codeHeight;
        codeLines = (uint16_t*) realloc(codeLines, codeLinesArraySize);
    }
}

void UpdateCode(uint16_t codeIndex) {
    static uint16_t prevIndex = -1;
    static uint16_t prevLine = -1;
    static uint16_t prevCol = -1;

    uint16_t line = 0;

    // Find the line of the current character through codeLines.
    while(codeLines[line] <= codeIndex && line < codeLinesAmount) {
        line++;
    }
    line--;

    // If we haven't printed this line before, scroll the window until it's visible.
    if(line >= codeLinesAmount - 1) {
        wprintw(outputWin, "char is %c\n", brainfuckCode[codeIndex]);
        wrefresh(outputWin);
        PrintNewLinesToIndex(codeIndex);
        line = codeLinesAmount - 2;
    }
    
    // Make the next code more readable.
    const int16_t row = line - currentCodeLine;
    const int16_t prevRow = prevLine - currentCodeLine;

    // If the line is out of bounds, but we've printed it before:
    if(row < 0 || row > codeHeight) {
        // Do thing hehe
        // return;
    }

    // Calculate the column of the character.
    uint16_t col = codeIndex - codeLines[line];

    // Print the previously selected character in the normal color.
    wattron(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[prevIndex])));
    mvwaddch(codeWin, prevRow, prevCol, brainfuckCode[prevIndex]);
    wattroff(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[prevIndex])));

    // Print the newly selected character in the selected color pair.
    wattron(codeWin, COLOR_PAIR(RUNNING_PAIR));
    mvwaddch(codeWin, row, col, brainfuckCode[codeIndex]);
    wattroff(codeWin, COLOR_PAIR(RUNNING_PAIR));

    prevIndex = codeIndex;
    prevLine = line;
    prevCol = col;

    wrefresh(codeWin);
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

// Output 1 character to the output window.
void OutputChar(char outChar) {
    wprintw(outputWin, "%c", outChar);
    wrefresh(outputWin);
}
