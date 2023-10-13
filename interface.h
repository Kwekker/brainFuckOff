#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <stdint.h>

void InitInterface(uint16_t memoryColumns, uint16_t outputHeight, char* code, uint8_t* memory);
void EndInterface(void);

void UpdateCode(uint16_t index);
void OutputChar(char outChar);

#endif // _INTERFACE_H_