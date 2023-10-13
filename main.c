#include <stdio.h>
#include <ncurses.h>

#include "interpreter.h"
#include "interface.h"

//TODO: Write window management before doing any brainfuck. It's both kinda fun.
//TODO: Design a window layout. 
//TODO: Write brainfuck interpreter.
//TODO: Decide how you're going to handle input.

int main(int argc, char *argv[]) {
    
    if(argc < 2) {
        fprintf(stderr, "\nNo input file provided.\n\n");
        return -1;
    }

    // Initialize the interpreter.
    char* brainfuckCode = InitInterpreter(argv[1], OutputChar);
    uint8_t* brainfuckMemory = GetMemory();

    if(brainfuckCode == NULL) {
        fprintf(stderr, "\nCould not open file.\n\n");
        return -1;
    }

    InitInterface(16, 32, brainfuckCode, brainfuckMemory);

    uint8_t running = 1;

    while(1) {
        if(running) {
            uint16_t* codeIndex;
            switch(InterpretNextChar(&codeIndex)) {
                case INTERPRETER_EOF:
                    running = 0;
                    break;

                case INTERPRETER_MEMORY_OUT_OF_BOUNDS:
                case INTERPRETER_OUT_OF_MEMORY:
                    return -1;
            }
            UpdateCode(codeIndex);
            getch();
        }
    }

    EndInterface();
    return 0;
}

