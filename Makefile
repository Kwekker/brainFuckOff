FILES= *.c

all:
	gcc $(FILES) -o brainfuckoff -Wall -lncurses

debug:
	gcc $(FILES) -o brainfuckoff -Wall -lncurses -g

run:
	./brainfuckoff