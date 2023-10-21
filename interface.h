#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <stdint.h>
#include "codeWindow.h"

#define DEBUG_WINDOW_HEIGHT 1
#define INPUT_WINDOW_HEIGHT 2

void InitInterface(uint16_t outputHeight, char* code);
void EndInterface(void);

void UpdateMemory(uint8_t* memory, uint16_t index);
void OutputChar(char outChar);

#endif // _INTERFACE_H_