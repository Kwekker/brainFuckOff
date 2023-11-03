#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <signal.h>

#include "interpreter.h"
#include "debugger.h"


// TODO: Debugger.
// TODO:    Input Handling.
// todo:    Reprint memory after not printing things.

static void sigHandler(int nerd);
static volatile uint8_t running = 1;

int main(int argc, char *argv[]) {
    signal(SIGINT, sigHandler);

    if(argc < 2) {
        fprintf(stderr, "\nNo input file provided.\n\n");
        return -1;
    }

    if(InitDebug(argv[1], 16)) {
        return EXIT_FAILURE;
    }

    while(running) {
        RunDebug();
    }

    EndDebug();
    printf("Fuck off :)\n");
    return 0;
}


static void sigHandler(int nerd) {
    running = 0;
}
