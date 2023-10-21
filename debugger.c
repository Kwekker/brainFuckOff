#include <stdio.h>
#include <ncurses.h>

#include "debugger.h"
#include "interpreter.h"
#include "interface.h"

#define KEY_SENTER 0x157


// TODO: Have some global 'break on breakpoint' and 'print output' variables that can be toggled.
// TODO: Otherwise there's going to be literally 165 different possible states.
enum state {
    STATE_IDLE,         // Does not run. The initial and default state.
    STATE_RUN,          // Runs while updating the display with the given delay (not implemented yet).
    STATE_RUN_FAST,     // Runs to next breakpoint.
    STATE_RUN_INDEF,    // Doesn't stop at breakpoint.
    STATE_STEP,         // Step one character.
    STATE_BIG_STEP,     // Just like a step but it skips all characters of the same type.
    STATE_EXIT_LOOP,        // Run up to the end of the current brackets or a breakpoint.
    STATE_EXIT_LOOP_FAST,   // Run up to the end of the current brackets or a breakpoint without updating the screen.
};

static enum state state = STATE_IDLE;
static void HandleKeyPress(int key);
static uint8_t inputRequested = 0;

// Keeps track of the initial loop depth when entering an 'exit loop' state.
static uint16_t initialLoopDepth = 0;

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

    // Enable mouse things (useful for setting breakpoints).
    mousemask(BUTTON1_CLICKED, NULL);


    InitInterface(outputHeight, brainfuckCode);

    return 0;
}

void EndDebug(void) {
    EndInterface();
}


void RunDebug(void) {
    MEVENT mouseEvent;

    // Toggling breakpoints with the mouse.
    if(getmouse(&mouseEvent) == OK) {
        uint16_t clickedIndex = InterfaceGetCodeIndex(mouseEvent.y, mouseEvent.x);
        ToggleBreakPointAtCodeIndex(clickedIndex);
        UpdateCode(clickedIndex, 0);
    }

    // TODO: Check if end of code has been reached.

    static enum state prevState = STATE_STEP;

    switch(state) {
        case STATE_IDLE: {
            if(prevState != STATE_IDLE) {
                UpdateCode(GetCodeIndex(), 1);
                UpdateMemory(GetMemory(), GetMemIndex());
            }

            int inKey = getch();
            if(inKey != ERR) HandleKeyPress(inKey); 
            break;
        }

        case STATE_STEP:
            InterpretNextChar(NULL);
            UpdateCode(GetCodeIndex(), 1);
            UpdateMemory(GetMemory(), GetMemIndex());
            state = STATE_IDLE;
            break;

        case STATE_BIG_STEP: {
            char nextChar;

            // Run until the character changes.
            char newChar = NO_BREAKPOINT_bm & InterpretNextChar(&nextChar);
            UpdateCode(GetCodeIndex(), 1);
            UpdateMemory(GetMemory(), GetMemIndex());

            if(nextChar != newChar || getch() != ERR) state = STATE_IDLE;

            break;
        }

        case STATE_RUN: {
            char nextChar;
            InterpretNextChar(&nextChar);
            UpdateCode(GetCodeIndex(), 1);
            UpdateMemory(GetMemory(), GetMemIndex());

            if(nextChar & BREAKPOINT_bm || getch() != ERR) state = STATE_IDLE;
            break;
        }

        case STATE_RUN_FAST: {
            char nextChar;
            InterpretNextChar(&nextChar);

            if(nextChar & BREAKPOINT_bm || getch() != ERR) state = STATE_IDLE;
            break;
        }

        case STATE_EXIT_LOOP: {
            char nextChar;
            InterpretNextChar(&nextChar);
            UpdateCode(GetCodeIndex(), 1);
            UpdateMemory(GetMemory(), GetMemIndex());

            if(nextChar & BREAKPOINT_bm || GetLoopDepth() < initialLoopDepth || getch() != ERR)
                state = STATE_IDLE;
        }
        case STATE_EXIT_LOOP_FAST: {
            char nextChar;
            InterpretNextChar(&nextChar);

            if(nextChar & BREAKPOINT_bm || GetLoopDepth() < initialLoopDepth || getch() != ERR)
                state = STATE_IDLE;
        }

        default:
            break;
    }

    prevState = state;
}

void HandleKeyPress(int key) {
    fprintf(stderr, "key %d\n", key);
    switch(key) {
        case ' ':
            state = STATE_RUN;
            break;
        
        case '\n':
            state = STATE_RUN_FAST;
            break;

        case KEY_RIGHT:
            state = STATE_STEP;
            break;

        case KEY_SRIGHT:
            state = STATE_BIG_STEP;
            break;

        case ']':
            state = STATE_EXIT_LOOP;
            initialLoopDepth = GetLoopDepth();
            break;
        
        case '}':
            state = STATE_EXIT_LOOP_FAST;
            initialLoopDepth = GetLoopDepth();
            break;

        case 'd':
            ToggleBreakPoint();
            UpdateCode(GetCodeIndex(), 0);
            break;
    }
}