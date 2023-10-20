#include <stdlib.h>

#include "codeWindow.h"
#include "color.h"

static uint16_t codeHeight = 0;
static WINDOW* codeWin;
static char* brainfuckCode;

// codeLines is an array that stores the first index of every line in the code. Lines include newlines, but also width overflows.
static uint16_t* codeLines;
static uint16_t codeLinesAmount;
static uint16_t codeLinesArraySize;
static uint16_t currentCodeLine = 0;

static void StoreCodeLine(uint16_t index);
static uint8_t GetCharColor(char c);
static void ReprintCode(void);
static void FindAllCodeLines(void);


// Print the first window of code.
void InitCodeWindow(WINDOW* window, char* code) {
    // Init the codeLines array with like idk 2 times the window height.
    codeLinesArraySize = 2 * codeHeight;
    codeLines = (uint16_t*) malloc(codeLinesArraySize * sizeof(uint16_t));


    codeWin = window;
    codeHeight = getmaxy(window);
    brainfuckCode = code;

    FindAllCodeLines();
    ReprintCode();

    wrefresh(codeWin);
}


void UpdateCode(uint16_t codeIndex) {
    static uint16_t prevIndex = -1;
    static uint16_t prevLine = -1;
    static uint16_t prevCol = -1;

    uint16_t line = 0;

    // Find the line of the current character through codeLines.
    do line++;
    while(codeLines[line] <= codeIndex && line < codeLinesAmount);
    line--;


    // Make the next code more readable.
    int16_t row = line - currentCodeLine;
    const int16_t prevRow = prevLine - currentCodeLine;

    // Print the previously selected character in the normal color.
    wattron(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[prevIndex])));
    mvwaddch(codeWin, prevRow, prevCol, brainfuckCode[prevIndex]);
    wattroff(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[prevIndex])));



    // If the line is out of bounds, but we've printed it before:
    // Scroll the screen down.
    if((row < CODE_TOP_MARGIN && line > CODE_TOP_MARGIN) || row < 0) {
        // Make sure that adding the margin does not put the first line lower than the top of the window.
        if(line <= CODE_TOP_MARGIN) currentCodeLine = 0;
        else currentCodeLine = line - CODE_TOP_MARGIN;

        ReprintCode();
    }

    // Scroll the screen up.
    else if(row > codeHeight - CODE_BOTTOM_MARGIN) {
        currentCodeLine = line - codeHeight + CODE_BOTTOM_MARGIN;
        ReprintCode();
    }


    // currentCodeLine could have changed.
    row = line - currentCodeLine;

    // Calculate the column of the character.
    int16_t col = codeIndex - codeLines[line];
    
    // Print the newly selected character in the selected color pair.
    wattron(codeWin, COLOR_PAIR(RUNNING_PAIR));
    mvwaddch(codeWin, row, col, brainfuckCode[codeIndex]);
    wattroff(codeWin, COLOR_PAIR(RUNNING_PAIR));

    prevIndex = codeIndex;
    prevLine = line;
    prevCol = col;

    wrefresh(codeWin);
}

void FindAllCodeLines(void) {
    uint16_t codeIndex = 0;
    scrollok(codeWin, 0);

    while(1) {
        // Move to the start of the final line so we get an error from waddch
        // when we try to print a character on the next line.
        wmove(codeWin, codeHeight - 1, 0);

        // Do this first so we store 0 as well (very nice to have).
        StoreCodeLine(codeIndex);

        int endOfLine = 0; // This is an int (cringe) because waddch returns an int.
        while(!endOfLine) {
            // Quit when we reach the end of the file.
            if(brainfuckCode[codeIndex] == '\0') {
                wmove(codeWin, 0, 0);
                wclear(codeWin);
                return;
            }

            // Print the line onto the window. There might be better ways to detect the end of the line,
            // but this is the easiest and most accessable one for me right now.
            endOfLine = waddch(codeWin, brainfuckCode[codeIndex]);
            codeIndex++;
        }
    }

    StoreCodeLine(codeIndex);
}

void StoreCodeLine(uint16_t index) {
    fprintf(stderr, "[%d] = %d\n", codeLinesAmount, index);
    codeLines[codeLinesAmount++] = index;

    // Reallocate codeLines if it too big.
    if(codeLinesAmount >= codeLinesArraySize) {
        codeLinesArraySize += codeHeight;
        codeLines = (uint16_t*) realloc(codeLines, codeLinesArraySize * sizeof(uint16_t));
    }
}


// Prints lines from `from` to `to` in lines (not rows).
void ReprintCode(void) {
    wclear(codeWin);
    wmove(codeWin, 0, 0);
    uint16_t codeIndex = codeLines[currentCodeLine];
    int endOfLine = 0;

    while(!endOfLine && brainfuckCode[codeIndex] != '\0') {
        wattron(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[codeIndex])));
        endOfLine = waddch(codeWin, brainfuckCode[codeIndex]);
        wattroff(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[codeIndex])));

        codeIndex++;
    }
}

uint16_t InterfaceGetCodeIndex(uint16_t mouseY, uint16_t mouseX) {
    // Return the index of the clicked character.
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
