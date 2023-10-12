
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
    char* brainfuckCode = InitInterpreter(argv[1], OutputPutChar);
    if(brainfuckCode == NULL) {
        fprintf(stderr, "\nCould not open file.\n\n");
        return -1;
    }

    InitInterface(16, 16, brainfuckCode);

    EndInterface();
    return 0;
}

