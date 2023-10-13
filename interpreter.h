#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_


#define INTERPRETER_OUT_OF_MEMORY -2
#define INTERPRETER_MEMORY_OUT_OF_BOUNDS -1
#define INTERPRETER_EOF 0


char* InitInterpreter(const char* inFile, void (*output)(char out));
char InterpretNextChar(uint16_t* codeIndex);
void ProvideInput(char input);

uint16_t GetCodeIndex(void);

// Returns a pointer to the memory array.
uint8_t* GetMemory(void);

#endif // _INTERPRETER_H_