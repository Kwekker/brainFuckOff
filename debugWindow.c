#include <string.h>

#include "debugWindow.h"
#include "color.h"

#define STATUS_COL 0
#define STATUS_LENGTH 20

#define INPUT_REQUEST_COL 22

WINDOW* debugWin;

// The struct that holds a piece of info displayed on the debug window
// (the cyan bar at the bottom of the screen).
typedef struct {
    char* name;
    uint16_t column;
    uint16_t valColumn;
    uint8_t size;
} element_t;

element_t elements[DEBUG_ELEMENT_AMOUNT] = {0};
uint8_t elIndex = 0;

void InitDebugWindow(WINDOW* win) {
    debugWin = win;

    wbkgd(debugWin, COLOR_PAIR(DEBUG_PAIR));
    wclear(debugWin);

    wrefresh(debugWin);
}

uint8_t NewDebugElement(char* name, uint8_t size) {
    // I love this feature so much.
    element_t el;


    if(elIndex > 0) {
        // +1 for the space between 2 elements.
        el.column = elements[elIndex - 1].column + elements[elIndex - 1].size + 1;
    }
    else el.column = 0;

    el.name = name;
    const uint8_t nameLen = strlen(name);

    // Calculate the size of the element in chars,
    // and the first column of the value.
    // +2 because of the ": ".
    el.valColumn = el.column + nameLen + 2;
    el.size = nameLen + 2 + size;
    
    elements[elIndex] = el; 
    
    mvwprintw(debugWin, 0, el.column, "%s: ", name);
    wrefresh(debugWin);

    return elIndex++;
}

void SetDebugElementBool(uint8_t index, uint8_t val) {
    const element_t el = elements[index];

    if(val) wattrset(debugWin, COLOR_PAIR(TRUE_PAIR));
    else wattrset(debugWin, COLOR_PAIR(FALSE_PAIR));

    mvwprintw(debugWin, 0, el.valColumn, "%s", val ? "True" : "False");
    wattrset(debugWin, 0);
    // Add an additional space if the value is true,
    // because "true" is one char shorter than "false".
    if(val) waddch(debugWin, ' ');

    wrefresh(debugWin);

}


void SetDebugElementInt(uint8_t index, uint16_t val) {
    
}


void SetDebugElementString(uint8_t index, const char* val) {
    
}
