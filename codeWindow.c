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
static void ReprintCodeLines(int16_t from, uint16_t to);
static void FindAllCodeLines(void);


// Print the first window of code.
void InitCodeWindow(WINDOW* window, char* code) {
    // Init the codeLines array with like idk 2 times the window height.
    codeLinesArraySize = 2 * codeHeight;
    codeLines = (uint16_t*) malloc(codeLinesArraySize * sizeof(uint16_t));


    codeWin = window;
    codeHeight = getmaxy(window);
    brainfuckCode = code;

    fprintf(stderr, "start %d high\n", codeHeight);

    FindAllCodeLines();
    ReprintCodeLines(0, codeHeight);

    wrefresh(codeWin);
    fprintf(stderr, "start\n");
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

    fprintf(stderr, "yes %d %d %d %d\n", codeIndex, row, line, currentCodeLine);


    // If the line is out of bounds, but we've printed it before:
    // Scroll the screen down.
    if((row < CODE_TOP_MARGIN && line > CODE_TOP_MARGIN) || row < 0) {
        const uint16_t prevCodeLine = currentCodeLine;

        // Make sure that adding the margin does not put the first line lower than the top of the window.
        if(line <= CODE_TOP_MARGIN) currentCodeLine = 0;
        else currentCodeLine = line - CODE_TOP_MARGIN;

        scrollok(codeWin, 1);
        wscrl(codeWin, currentCodeLine - prevCodeLine);
        scrollok(codeWin, 0);

        ReprintCodeLines(0, prevCodeLine - currentCodeLine);
    }

    // Scroll the screen up.
    else if(row > codeHeight - CODE_BOTTOM_MARGIN) {
        const uint16_t prevCodeLine = currentCodeLine;

        currentCodeLine = line - codeHeight + CODE_BOTTOM_MARGIN;

        scrollok(codeWin, 1);
        wscrl(codeWin, currentCodeLine - prevCodeLine);
        scrollok(codeWin, 0);

        ReprintCodeLines(prevCodeLine + codeHeight - currentCodeLine, codeHeight);
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
}


void StoreCodeLine(uint16_t index) {
    fprintf(stderr, "codeLines[%d] = %d;\n", codeLinesAmount, index);
    codeLines[codeLinesAmount++] = index;

    // Reallocate codeLines if it too big.
    if(codeLinesAmount >= codeLinesArraySize) {
        codeLinesArraySize += codeHeight;
        fprintf(stderr, "new size %d", codeLinesArraySize);
        codeLines = (uint16_t*) realloc(codeLines, codeLinesArraySize * sizeof(uint16_t));
    }
}


// Prints lines from `from` to `to` in rows (not lines).
void ReprintCodeLines(int16_t from, uint16_t to) {
    // Make sure to not try to print outside the code window.
    if(from < 0) from = 0;
    if(to > codeLinesAmount) to = codeLinesAmount - 1;
    wmove(codeWin, from, 0);

    for(uint16_t i = codeLines[currentCodeLine + from]; i < codeLines[currentCodeLine + to]; i++) {
        if(brainfuckCode[i] == '\0') return;

        wattron(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[i])));
        int err = waddch(codeWin, brainfuckCode[i]);
        wattroff(codeWin, COLOR_PAIR(GetCharColor(brainfuckCode[i])));


        if(err) return;
    }

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
