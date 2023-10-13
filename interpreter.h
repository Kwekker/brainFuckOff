#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_


#define INTERPRETER_OUT_OF_MEMORY -2
#define INTERPRETER_MEMORY_OUT_OF_BOUNDS -1
#define INTERPRETER_EOF 0


char* InitInterpreter(const char* inFile, void (*output)(char out));
char InterpretNextChar(void);
void ProvideInput(char input);

#endif // _INTERPRETER_H_