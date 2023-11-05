# brainFuckOff
Yet another brainfuck debugger/interpreter.

This one provides an ncurses interface which can be used to debug.
For now the building is as follows:
```sh
make
```
And running:
```sh
./brainFuckOff brainfuckfile 2> something.log
```
The `2>` is to redirect `stderr` into a log file, because I still have some debug printfs in the code, and you don't want that messing up the ncurses interface.
It is, of course, very cringe to have this in the main branch, but I kinda forgot to start a develop branch. I'll do so whenever I decide to remove the printfs.
