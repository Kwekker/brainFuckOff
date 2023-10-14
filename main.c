#include <stdio.h>
#include <ncurses.h>

#include "interpreter.h"
#include "interface.h"

// TODO: Window scrolling.
// TODO: Debugger.

int main(int argc, char *argv[]) {
    
    if(argc < 2) {
        fprintf(stderr, "\nNo input file provided.\n\n");
        return -1;
    }

    // Initialize the interpreter.
    char* brainfuckCode = InitInterpreter(argv[1], OutputChar);
    if(brainfuckCode == NULL) {
        fprintf(stderr, "\nCould not open file.\n\n");
        return -1;
    }


    uint8_t running = 1;

    // Start the cursor up at the first valid Brainfuck character.
    InitInterface(35, brainfuckCode);
    UpdateCode(GetCodeIndex());

    while(1) {
        if(running) {
            switch(InterpretNextChar()) {
                case '#':
                    getch();
                    break;

                case ',':
                    break;

                case INTERPRETER_EOF:
                    running = 0;
                    break;

                case INTERPRETER_MEMORY_OUT_OF_BOUNDS:
                case INTERPRETER_OUT_OF_MEMORY:
                    return -1;
            }
            UpdateCode(GetCodeIndex());
            UpdateMemory(GetMemory(), GetMemIndex());
        }
    }

    EndInterface();
    return 0;
}

