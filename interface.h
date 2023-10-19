#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include <stdint.h>

// The amount of extra lines to scroll down
// when the code pointer is above the current code screen.
#define CODE_TOP_MARGIN 2

// The amount of extra lines to scroll up
// when the code pointer is under the current code screen.
#define CODE_BOTTOM_MARGIN 2



void InitInterface(uint16_t outputHeight, char* code);
void EndInterface(void);

void UpdateCode(uint16_t index);
void UpdateMemory(uint8_t* memory, uint16_t index);
void OutputChar(char outChar);

#endif // _INTERFACE_H_