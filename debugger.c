#include <stdio.h>
#include <ctype.h>
#include <ncurses.h>

#include "debugger.h"
#include "interpreter.h"
#include "interface.h"
#include "color.h"

#define KEY_SENTER 0x157

#define INPUT_BUFFER_SIZE 1024


enum State {
    STATE_IDLE,         // Does not run. The initial and default state.
    STATE_RUN,          // Runs while updating the display with the given delay (not implemented yet).
    STATE_BIG_STEP,     // Skips all characters of the same type.
    STATE_EXIT_LOOP,    // Run up to the end of the current brackets or a breakpoint.
};

// I really like this C feature.
const char* stateNames[] = {
    [STATE_IDLE]            = "idle",
    [STATE_RUN]             = "run",
    [STATE_BIG_STEP]        = "step",
    [STATE_EXIT_LOOP]       = "exit loop",
};


// The mode determines where the user input goes.
enum Mode { 
    MODE_DEBUG, // Default.
    MODE_INPUT,
    MODE_MENU
};

static const char *modeNames[] = {
    [MODE_DEBUG] = "debug",
    [MODE_INPUT] = "input",
    [MODE_MENU]  = "menu",
};


static void HandleDebugKeyPress(int key);
static int16_t InputRequested(void);
static void InputPutChar(char put);
static void HandleTypedInputChar(int typedChar);

static enum State state = STATE_IDLE;
static enum Mode mode = MODE_DEBUG;
static uint8_t stateDebugElement = 0;
static uint8_t modeDebugElement = 0;

static char inputBuffer[INPUT_BUFFER_SIZE]; // I really do not feel like making this dynamic.
static uint16_t inputBufferSize = 0;

static uint8_t printOutput = 1; // Replace these with bools in C23 (not using stdbool lmao).
static uint8_t breakOnBreakpoint = 1;
static uint8_t printOutputDebugElement = 0;
static uint8_t breakOnBreakpointDebugElement = 0;

// waitForInput gets set whenever a ',' is encountered, but there is no input.
// The program will halt, but remember it's current state,
// so that it can continue when the user has provided their input.
static uint8_t waitForInput = 0;

static uint8_t interpretError = 0;

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
    
    // Init the debug window.
    printOutputDebugElement = NewDebugElement("Visual", 5);
    SetDebugElementBool(printOutputDebugElement, printOutput);

    breakOnBreakpointDebugElement = NewDebugElement("Breakpoints", 5);
    SetDebugElementBool(breakOnBreakpointDebugElement, breakOnBreakpoint);

    stateDebugElement = NewDebugElement("State", 9);
    modeDebugElement = NewDebugElement("Mode", 5);

    return 0;
}

void EndDebug(void) {
    EndInterface();
}


void RunDebug(void) {
    MEVENT mouseEvent;

    // Toggling breakpoints with the mouse.
    if(getmouse(&mouseEvent) == OK) {
        int16_t clickedIndex = InterfaceGetCodeIndex(mouseEvent.y, mouseEvent.x);

        if(clickedIndex != ERR) {
            ToggleBreakPointAtCodeIndex(clickedIndex);
            // This actually scrolls the window if you click too low, which is actually kinda nice, so I'll call it a feature.
            // For some reason it also doesn't scroll away from the cursor, which is doubly nice.
            UpdateCode(clickedIndex, 0);
        }
    }

    // TODO: Check if end of code has been reached.
    // TODO: Make input mode only available if input is in a seperate window.

    char nextChar = '\0';
    char prevChar;
    if(state != STATE_IDLE && !waitForInput) prevChar = InterpretNextChar(&nextChar);

    // Handle errors.
    if(INTERPRETER_IS_ERROR(prevChar)) {
        interpretError = prevChar;
        switch(prevChar) {

        case INTERPRETER_EOF: 
            PrintInfoMessage("Finished running!", COLOR_PAIR(SUCCESS_PAIR));
            break;
        
        case INTERPRETER_MEMORY_OUT_OF_BOUNDS: 
            PrintInfoMessage("Error: Tried to go out of bounds.", COLOR_PAIR(ERROR_PAIR));
            break;
        
        case INTERPRETER_OUT_OF_MEMORY: 
            PrintInfoMessage("Error: Out of memory! Buy more RAM!", COLOR_PAIR(ERROR_PAIR));
            break;
        
        case INTERPRETER_LEAK: 
            PrintInfoMessage("Bro I don't even know when this error happens.", COLOR_PAIR(ERROR_PAIR));
            break;
            
        }
    }

    // Handle special states.
    if(!waitForInput && !interpretError) {
        switch(state) {

        case STATE_BIG_STEP:
            // Run until the character changes.
            const char prevChar = NO_BREAKPOINT_bm & InterpretNextChar(&nextChar);
            if(prevChar != nextChar) state = STATE_IDLE;
            break;

        case STATE_EXIT_LOOP:
            // Run until the loop depth is lower than the loop depth when entering this state.
            if(GetLoopDepth() < initialLoopDepth) state = STATE_IDLE;
            break;

        default: break;

        }
    }

    // Keep track of state changes.
    static enum State prevState = STATE_RUN;

    // Break on breakpoints if it's enabled.
    if(breakOnBreakpoint && (nextChar & BREAKPOINT_bm))
        state = STATE_IDLE;

    // Print the output if it's enabled and we just did something.
    if(printOutput && prevState != STATE_IDLE) {
        UpdateCode(GetCodeIndex(), 1);
        UpdateMemory(GetMemory(), GetMemIndex());
    }

    // Handle key presses.
    int inKey = getch();
    if(inKey != ERR) {
        switch(mode) {
        case MODE_DEBUG: 
            HandleDebugKeyPress(inKey); 
            break;
        case MODE_INPUT: 
            HandleTypedInputChar(inKey); 
            break;
        case MODE_MENU:
            break;
        }
    }


    if(state != prevState) {
        // Gotta update the interface when coming out of a state that doesn't.
        if(state == STATE_IDLE) {
            // Set timeout to block to spare resources.
            timeout(-1);
            UpdateCode(GetCodeIndex(), 1);
            UpdateMemory(GetMemory(), GetMemIndex());
        }
        else timeout(0);

        SetDebugElementString(stateDebugElement, stateNames[state]);


        prevState = state;
    }

    // Keep track of mode changes.
    static enum Mode prevMode = MODE_MENU;
    if(mode != prevMode) {
        SetDebugElementString(modeDebugElement, modeNames[mode]);

        prevMode = mode;
    }

}

void HandleDebugKeyPress(int key) {

    switch(key) {
    case 'i':
        mode = MODE_INPUT;
        curs_set(1);
        break;

    case 'b':
        ToggleBreakPoint();
        UpdateCode(GetCodeIndex(), 0);
        break;

    case ' ':
        if(state == STATE_IDLE) state = STATE_RUN;
        else state = STATE_IDLE;
        break;

    case KEY_RIGHT:
        // Step to the next character.
        InterpretNextChar(NULL);
        UpdateCode(GetCodeIndex(), 1);
        UpdateMemory(GetMemory(), GetMemIndex());
        break;

    case KEY_SRIGHT:
        state = STATE_BIG_STEP;
        break;

    case ']':
        state = STATE_EXIT_LOOP;
        initialLoopDepth = GetLoopDepth();
        break;

    case 's':
        printOutput = !printOutput;
        SetDebugElementBool(printOutputDebugElement, printOutput);
        break;

    case 'B':
        breakOnBreakpoint = !breakOnBreakpoint;
        SetDebugElementBool(breakOnBreakpointDebugElement, breakOnBreakpoint);
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
        mode = MODE_DEBUG;
        waitForInput = 0;
        curs_set(0);
        
        // Set getch() back to non-blocking so we can continue to runwithout needing an input at every command.
        if(state != STATE_IDLE) timeout(0);

        break;

    case '\e':
    case '\t':
        // Tab is also used to exit input mode
        // because apparently the escape key blocks the program for an entire second.
        state = STATE_IDLE;
        mode = MODE_DEBUG;
        waitForInput = 0;
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

    waitForInput = 1;
    mode = MODE_INPUT;
    
    // Make getch blocking to spare resources.
    timeout(-1);

    return -1;
}