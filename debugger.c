#include <stdio.h>
#include <ncurses.h>

#include "debugger.h"
#include "interpreter.h"
#include "interface.h"

#define KEY_SENTER 0x157

enum state {
    STATE_IDLE,         // Does not run. The initial and default state.
    STATE_RUN,          // Runs to next debug char.
    STATE_SLOW_RUN,     // Runs while updating the display with the given delay (not implemented yet).
    STATE_RUN_INDEF,    // Doesn't stop at debug chars.
    STATE_STEP,         // Step one character.
    STATE_BIG_STEP,     // Just like a step but it skips all characters of the same type.
};

static enum state state = STATE_IDLE;
static void SwitchStateByKey(int key);
static uint8_t inputRequested = 0;

uint8_t InitDebug(const char* brainfuckFile, uint16_t outputHeight) {
    
    // Initialize the interpreter.
    char* brainfuckCode = InitInterpreter(brainfuckFile, &inputRequested, OutputChar);
    if(brainfuckCode == NULL) {
        fprintf(stderr, "\nCould not open/read file.\n\n");
        return -1;
    }
    
    // Setup ncurses
    initscr();
    noecho();       // Don't echo input.
    curs_set(0);    // Hide cursor.
    timeout(0);     // Non-blocking inputs please.
    cbreak();
    keypad(stdscr, 1);
    refresh();      // Refresh the screen once before doing anything.
    getch();        // Get rid of the initial \n.

    InitInterface(outputHeight, brainfuckCode);

    return 0;
}

void EndDebug(void) {
    EndInterface();
}


void RunDebug(void) {

    switch(state) {
        case STATE_IDLE: {
            UpdateCode(GetCodeIndex());
            UpdateMemory(GetMemory(), GetMemIndex());

            int inKey = getch();
            if(inKey != ERR) SwitchStateByKey(inKey); 
            break;
        }

        case STATE_STEP:
            InterpretNextChar(NULL);
            UpdateCode(GetCodeIndex());
            UpdateMemory(GetMemory(), GetMemIndex());
            state = STATE_IDLE;
            break;

        case STATE_BIG_STEP: {
            char nextChar;

            // Run until the character changes.
            char newChar = InterpretNextChar(&nextChar);
            UpdateCode(GetCodeIndex());
            UpdateMemory(GetMemory(), GetMemIndex());

            if(nextChar != newChar || getch() != ERR) state = STATE_IDLE;

            break;
        }

        case STATE_SLOW_RUN: {
            char nextChar;
            InterpretNextChar(&nextChar);
            UpdateCode(GetCodeIndex());
            UpdateMemory(GetMemory(), GetMemIndex());

            if(nextChar == '#' || getch() != ERR) state = STATE_IDLE;
            break;
        }

        case STATE_RUN: {
            char nextChar;
            InterpretNextChar(&nextChar);

            if(nextChar == '#' || getch() != ERR) state = STATE_IDLE;
            break;
        }

        default:
            break;
    }

}

void SwitchStateByKey(int key) {
    fprintf(stderr, "key %d\n", key);
    switch(key) {
        case ' ':
            state = STATE_SLOW_RUN;
            break;
        
        case '\n':
            state = STATE_RUN;
            break;

        case KEY_SENTER:
            state = STATE_RUN_INDEF;
            break;

        case KEY_RIGHT:
            state = STATE_STEP;
            break;

        case KEY_SRIGHT:
            state = STATE_BIG_STEP;
            break;
    }
}