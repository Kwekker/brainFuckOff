#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_


// Characters become break
#define BREAKPOINT_bm    0x80
#define NO_BREAKPOINT_bm 0x7f

#define INTERPRETER_EOF -1
#define INTERPRETER_MEMORY_OUT_OF_BOUNDS -2
#define INTERPRETER_OUT_OF_MEMORY -3
#define INTERPRETER_LEAK -4

#define INTERPRETER_IS_ERROR(returnedChar) (returnedChar < 0 && returnedChar >= -4)

char* InitInterpreter(const char* inFile, uint8_t* requestInput, void (*output)(char out));
char InterpretNextChar(char* nextChar);
void ProvideInput(uint8_t input);

uint16_t GetCodeIndex(void);
uint16_t GetMemIndex(void);
uint16_t GetLoopDepth(void);
void ToggleBreakPoint(void);
void ToggleBreakPointAtCodeIndex(uint16_t codeIndex);

// Returns a pointer to the memory array.
uint8_t* GetMemory(void);

#endif // _INTERPRETER_H_