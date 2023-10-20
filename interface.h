#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <stdint.h>
#include "codeWindow.h"

void InitInterface(uint16_t outputHeight, char* code);
void EndInterface(void);

void UpdateCode(uint16_t index);
void UpdateMemory(uint8_t* memory, uint16_t index);
void OutputChar(char outChar);

uint16_t InterfaceGetCodeIndex(uint16_t mouseY, uint16_t mouseX);

#endif // _INTERFACE_H_