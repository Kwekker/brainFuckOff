FILES= *.c

all:
	gcc $(FILES) -o brainfuckoff -Wall -lncurses

run:
	./brainfuckoff