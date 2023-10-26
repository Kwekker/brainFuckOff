#include <stdio.h>
#include <ctype.h>
#include <ncurses.h>

#include "debugger.h"
#include "interpreter.h"
#include "interface.h"

#define KEY_SENTER 0x157

#define INPUT_BUFFER_SIZE 1024


// TODO: Have some global 'break on breakpoint' and 'print output' variables that can be toggled.
// TODO: Otherwise there's going to be literally 165 different possible states.
enum State {
    STATE_IDLE,         // Does not run. The initial and default state.
    STATE_INPUT,        // Activates when the input key is pressed, stops when escape is pressed.
    STATE_RUN,          // Runs while updating the display with the given delay (not implemented yet).
    STATE_RUN_FAST,     // Runs to next breakpoint.
    STATE_RUN_INDEF,    // Doesn't stop at breakpoint.
    STATE_STEP,         // Step one character.
    STATE_BIG_STEP,     // Just like a step but it skips all characters of the same type.
    STATE_EXIT_LOOP,        // Run up to the end of the current brackets or a breakpoint.
    STATE_EXIT_LOOP_FAST,   // Run up to the end of the current brackets or a breakpoint without updating the screen.


};

// I really like this C feature
const char* stateNames[] = {
    [STATE_IDLE]            = "idle",
    [STATE_INPUT]           = "input",
    [STATE_RUN]             = "running",
    [STATE_RUN_FAST]        = "running",
    [STATE_RUN_INDEF]       = "running",
    [STATE_STEP]            = "stepping",
    [STATE_BIG_STEP]        = "stepping",
    [STATE_EXIT_LOOP]       = "exiting loop",
    [STATE_EXIT_LOOP_FAST]  = "exiting loop",
};

static void HandleKeyPress(int key);
static int16_t InputRequested(void);
void InputPutChar(char put);
void HandleTypedInputChar(int typedChar);

static enum State state = STATE_IDLE;
static char inputBuffer[INPUT_BUFFER_SIZE]; // I really do not feel like making this dynamic.
static uint16_t inputBufferSize = 0;
static uint8_t isInputRequested = 0;

// Keeps track of the initial loop depth when entering an 'exit loop' state.
static uint16_t initialLoopDepth = 0;

uint8_t InitDebug(const char* brainfuckFile, uint16_t outputHeight) {
    
    // Initialize the interpreter.
    char* brainfuckCode = InitInterpreter(brainfuckFile, OutputChar, InputRequested);
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
    // TODO: Keep running while input is being entered??
    // TODO: At least something to make inputting a little bit less painful


    switch(state) {
    case STATE_IDLE: {
        int inKey = getch();
        if(inKey != ERR) HandleKeyPress(inKey); 
        break;
    }

    case STATE_INPUT: {
        int typedChar = getch();
        HandleTypedInputChar(typedChar);

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

        if(nextChar != newChar || getch() == ' ') state = STATE_IDLE;

        break;
    }

    case STATE_RUN: {
        char nextChar;
        InterpretNextChar(&nextChar);
        UpdateCode(GetCodeIndex(), 1);
        UpdateMemory(GetMemory(), GetMemIndex());

        if(nextChar & BREAKPOINT_bm || getch() == ' ') state = STATE_IDLE;
        break;
    }

    case STATE_RUN_FAST: {
        char nextChar;
        InterpretNextChar(&nextChar);

        if(nextChar & BREAKPOINT_bm || getch() == ' ') state = STATE_IDLE;
        break;
    }

    case STATE_EXIT_LOOP: {
        char nextChar;
        InterpretNextChar(&nextChar);
        UpdateCode(GetCodeIndex(), 1);
        UpdateMemory(GetMemory(), GetMemIndex());

        if(nextChar & BREAKPOINT_bm || GetLoopDepth() < initialLoopDepth || getch() == ' ')
            state = STATE_IDLE;
        break;
    }
    case STATE_EXIT_LOOP_FAST: {
        char nextChar;
        InterpretNextChar(&nextChar);

        if(nextChar & BREAKPOINT_bm || GetLoopDepth() < initialLoopDepth || getch() == ' ')
            state = STATE_IDLE;
        break;
    }

    default:
        break;
    }

    static enum State prevState = STATE_STEP;

    if(state != prevState) {
        SetDebugStatus(stateNames[state]);

        // Gotta update the interface when coming out of a state that doesn't.
        if(state == STATE_IDLE) {
            UpdateCode(GetCodeIndex(), 1);
            UpdateMemory(GetMemory(), GetMemIndex());
        }

        prevState = state;
    }

}

void HandleKeyPress(int key) {

    // This switch statement does get run when an input is required, the next one does not.
    // This is because you can't really start running when no input is provided. 
    switch(key) {
    case 'i':
        state = STATE_INPUT;
        curs_set(1);
        break;

    case 'd':
        ToggleBreakPoint();
        UpdateCode(GetCodeIndex(), 0);
        break;
    }

    if(isInputRequested) return;

    switch(key) {
    case ' ':
        state = STATE_RUN;
        SetDebugStatus("running");
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
        SetDebugStatus("exiting loop");
        break;
    
    case '}':
        state = STATE_EXIT_LOOP_FAST;
        initialLoopDepth = GetLoopDepth();
        break;
    }
}

void HandleTypedInputChar(int typedChar) {

    if(typedChar != ERR) fprintf(stderr, "key %x\n", typedChar);

    switch(typedChar) {
    case ERR: break;

    // Switch to idle if either Enter or Escape is pressed.
    case '\n':
        InputPutChar('\n');
        // Notice the lack of a break;
    case '\t':
        // Tab is used to exit input mode because apparently the escape key blocks the program for an entire second.
        if(inputBufferSize) isInputRequested = 0;
        DebugInputRequested(0);
        state = STATE_IDLE;
        curs_set(0);
        break;
    
    // Do not switch to idle if Shift + Enter is pressed.
    case KEY_SENTER:
        InputPutChar('\n');
        break;

    // For some fucking reason backspaces are given as 0x07 (which is BELL, btw)
    // so I'll just list all the possibilities, just in case.
    case 0x7:
    case '\b':
    case KEY_BACKSPACE:
        InputPutChar('\b');
        break;

    default:
        InputPutChar(typedChar);
        break;
    }
}

void InputPutChar(char put) {
    if(inputBufferSize >= INPUT_BUFFER_SIZE) return;

    if(put == '\b') {

        // TODO: Check if it's a return in the buffer and handle that so that it doesn't teleport you to the end of a line that never was at the end of a line.
        if(!inputBufferSize) return;
        for(uint16_t i = 0; i < inputBufferSize - 1; i++) {
            inputBuffer[i] = inputBuffer[i + 1];
        }
        inputBufferSize--;
        OutputBackspaceChar();
        return;
    }

    fprintf(stderr, "in is %02x which is %x but does it run?\n", put, isprint(put));

    if(!isprint(put) && put != '\n') return;

    fprintf(stderr, "it does :)\n");

    // Move the entire buffer to the right.
    // (I couldn't be bothered with a circular buffer.)
    for(uint16_t i = inputBufferSize; i > 0; i--) {
        inputBuffer[i] = inputBuffer[i - 1];
    }

    inputBuffer[0] = put;
    inputBufferSize++;
    OutputChar(put);
}

int16_t InputRequested(void) {

    if(inputBufferSize) return inputBuffer[--inputBufferSize];

    DebugInputRequested(1);
    isInputRequested = 1;
    state = STATE_IDLE;
    return -1;
}