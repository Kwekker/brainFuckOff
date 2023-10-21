FILES= *.c

all:
	gcc $(FILES) -o brainfuckoff -Wall -lncurses

fast:
	gcc $(FILES) -o brainfuckoff -Wall -lncurses -Ofast

debug:
	gcc $(FILES) -o brainfuckoff -Wall -lncurses -g

run:
	./brainfuckoff