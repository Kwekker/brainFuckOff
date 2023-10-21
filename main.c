#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>

#include "interpreter.h"
#include "debugger.h"


// TODO: Debugger.
// TODO:    Input Handling.
// todo:    Reprint memory after not printing things.

void outputThing(char c) {
    printf("%c", c);
}

int main(int argc, char *argv[]) {
    
    if(argc < 2) {
        fprintf(stderr, "\nNo input file provided.\n\n");
        return -1;
    }

    if(InitDebug(argv[1], 16)) {
        return EXIT_FAILURE;
    }

    while(1) {
        RunDebug();
    }

    EndDebug();
    return 0;
}
