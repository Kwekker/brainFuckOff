#include <stdlib.h>

#include "codeWindow.h"
#include "color.h"
#include "interpreter.h" // For the defines

static uint16_t codeHeight = 0;
static WINDOW* codeWin;
static char* brainfuckCode;

// codeLines is an array that stores the first index of every line in the code. Lines include newlines, but also width overflows.
static uint16_t* codeLines;
static uint16_t codeLinesAmount;
static uint16_t codeLinesArraySize;
static uint16_t currentCodeLine = 0;

static void StoreCodeLine(uint16_t index);
static int GetCharAttr(char c);
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

// Reprints a character at a certain index, and selects it as the new cursor if `isCursor` != 0.
void UpdateCode(uint16_t codeIndex, uint8_t isCursor) {
    static uint16_t prevIndex = -1;
    static uint16_t prevLine = -1;
    static uint16_t prevCol = -1;


    //* Finding the line //

    uint16_t line = 0;

    // Find the line of the character that has to be reprinted through codeLines.
    do line++;
    while(codeLines[line] <= codeIndex && line < codeLinesAmount);
    line--;


    // Make the next code more readable.
    int16_t row = line - currentCodeLine;
    const int16_t prevRow = prevLine - currentCodeLine;


    //* Scrolling //

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

    // If we're reprinting a new cursor, we want to remove the old one.
    // Doesn't need to happen if the window was reprinted by scrolling.
    else if(isCursor) {
        // Print the previously selected character in the normal color.
        wattrset(codeWin, GetCharAttr(brainfuckCode[prevIndex]));
        mvwaddch(codeWin, prevRow, prevCol, NO_BREAKPOINT_bm & brainfuckCode[prevIndex]);
    }

    // currentCodeLine could have changed.
    row = line - currentCodeLine;

    // Calculate the column of the character.
    int16_t col = codeIndex - codeLines[line];
    

    //* Printing //

    // Set the character's color & style.
    if(isCursor) {
        // Select the character if we're reprinting a new cursor.
        wattrset(codeWin, COLOR_PAIR(RUNNING_PAIR));
        // Add an underline if it's a breakpoint.
        if(brainfuckCode[codeIndex] & BREAKPOINT_bm) wattron(codeWin, A_UNDERLINE);

        // Keep track of the previous cursor position.
        prevIndex = codeIndex;
        prevLine = line;
        prevCol = col;
    }
        // Color the character normally we're reprinting a new breakpoint.
    else wattrset(codeWin, GetCharAttr(brainfuckCode[codeIndex]));

    // Actually print the character.
    mvwaddch(codeWin, row, col, NO_BREAKPOINT_bm & brainfuckCode[codeIndex]);

    wstandend(codeWin);
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
            endOfLine = waddch(codeWin, NO_BREAKPOINT_bm & brainfuckCode[codeIndex]);
            codeIndex++;
        }
    }

    StoreCodeLine(codeIndex);
}

void StoreCodeLine(uint16_t index) {
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
        wattrset(codeWin, GetCharAttr(brainfuckCode[codeIndex]));
        endOfLine = waddch(codeWin, NO_BREAKPOINT_bm & brainfuckCode[codeIndex]);

        codeIndex++;
    }
    wstandend(codeWin);
}

uint16_t InterfaceGetCodeIndex(uint16_t mouseY, uint16_t mouseX) {
    // Return the index of the clicked character.
    mouseY -= getbegy(codeWin);
    mouseX -= getbegx(codeWin);


    const uint16_t line = mouseY + currentCodeLine;
    const uint16_t index = codeLines[line] + mouseX;

    // If it's not on a character, return an error.
    if(index >= codeLines[line + 1]) return ERR;

    return index;
}

int GetCharAttr(char c) {

    int underline = 0;
    // Put an underline if it's a breakpoint character
    if(c & BREAKPOINT_bm) underline = A_UNDERLINE;

    switch(c & NO_BREAKPOINT_bm) {
    case '[':
    case ']':
        return underline | COLOR_PAIR(BRACKET_PAIR);

    case '>':
    case '<':
        return underline | COLOR_PAIR(MOVER_PAIR);

    case '+':
    case '-':
        return underline | COLOR_PAIR(CHANGER_PAIR);

    case '.':
    case ',':
        return underline | COLOR_PAIR(INOUT_PAIR);

    case '#':
        return underline | COLOR_PAIR(BREAKPOINT_PAIR);

    default:
        return COLOR_PAIR(COMMENT_PAIR);
    }

}
