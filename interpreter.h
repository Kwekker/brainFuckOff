#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_


#define INTERPRETER_OUT_OF_MEMORY -2
#define INTERPRETER_MEMORY_OUT_OF_BOUNDS -1
#define INTERPRETER_EOF 0


char* InitInterpreter(const char* inFile, void (*output)(char out));
char InterpretNextChar(void);
void ProvideInput(uint8_t input);

uint16_t GetCodeIndex(void);
uint16_t GetMemIndex(void);

// Returns a pointer to the memory array.
uint8_t* GetMemory(void);

#endif // _INTERPRETER_H_