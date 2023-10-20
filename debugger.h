#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <stdint.h>

uint8_t InitDebug(const char* brainfuckFile, uint16_t outputHeight);
void RunDebug(void);
void EndDebug(void);

#endif // _DEBUG_H_