#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <ncurses.h>

char* InitInterpreter(const char* inFile, void (*output)(char out));
char InterpretNextChar(void);
void ProvideInput(char input);

#endif // _INTERPRETER_H_