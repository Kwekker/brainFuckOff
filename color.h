#ifndef _COLOR_H_
#define _COLOR_H_



// Brainfuck character color pairs
#define BRACKET_PAIR    1
#define MOVER_PAIR      2
#define CHANGER_PAIR    3
#define INOUT_PAIR      4
#define COMMENT_PAIR    5
#define RUNNING_PAIR    6

#define MEM_IDLE_PAIR   7
#define MEM_CURSOR_PAIR 8
#define MEM_USED_PAIR   9
#define MEM_INDEX_PAIR  10

#define COLOR_GREY      8


void InitColorPairs(void);

#endif // _COLOR_H_