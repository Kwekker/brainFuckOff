#ifndef _DEBUGWINDOW_H_
#define _DEBUGWINDOW_H_

#include <stdint.h>
#include <ncurses.h>

#define DEBUG_ELEMENT_AMOUNT 5

void InitDebugWindow(WINDOW* win);
void SetDebugStatus(const char* newStatus);
void DebugInputRequested(uint8_t requested);

uint8_t NewDebugElement(char* name, uint8_t size);
void SetDebugElementBool(uint8_t index, uint8_t val);
void SetDebugElementInt(uint8_t index, uint16_t val);
void SetDebugElementString(uint8_t index, const char* val);

#endif // _DEBUGWINDOW_H_