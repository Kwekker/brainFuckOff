#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "interpreter.h"
#include "interface.h"

#define KEY_CHARACTERS "#[]+-<>.,"

// I don't feel like doing this dynamically.
#define MAX_LOOP_DEPTH 256
#define INITIAL_MEMORY_SIZE 256

static uint16_t memorySize = INITIAL_MEMORY_SIZE;
static uint8_t* memory;
static uint16_t memoryIndex;

static char* fullCode;
static char* strippedCode;
static uint16_t* strippedIndeces;
static uint16_t strippedLength = 0;
static uint16_t strippedIndex = 0;

static void (*OutFunction)(char out);

static char *FileToBuffer(const char *fileName);
static uint16_t StripCode(char* code);
static uint8_t* requestInput;


char* InitInterpreter(const char* inFileName, uint8_t* inputRequested, void (*Output)(char out)) {
    fullCode = FileToBuffer(inFileName);
    if(fullCode == NULL) return NULL;
    strippedLength = StripCode(fullCode);
   
    OutFunction = Output;
    requestInput = inputRequested;

    memory = (uint8_t*)malloc(INITIAL_MEMORY_SIZE);
    memset(memory, 0, memorySize);

    return fullCode;
}


char InterpretNextChar(char* nextChar) {

    // Store the char because the index might change.
    char codeChar = strippedCode[strippedIndex];

    switch(codeChar) {
        case '+':
            memory[memoryIndex]++;
            break;

        case '-':
            memory[memoryIndex]--;
            break;

        case '>':
            memoryIndex++;
            // Allocate more memory when needed.
            if(memoryIndex >= memorySize) {
                memorySize += INITIAL_MEMORY_SIZE;
                memory = (uint8_t*)realloc(memory, memorySize);

                if(memory == NULL) {
                    fprintf(stderr, "Not enough memory :(\n");
                    return INTERPRETER_OUT_OF_MEMORY;
                }

                // Fill it with 0's.
                memset(memory + memorySize - INITIAL_MEMORY_SIZE, 0x00, INITIAL_MEMORY_SIZE);
            }
            break;

        case '<':
            if(memoryIndex == 0) {
                return INTERPRETER_MEMORY_OUT_OF_BOUNDS;
            }
            memoryIndex--;
            break;

        case '.':
            OutFunction(memory[memoryIndex]);
            break;

        case ',':
            // The calling function should now consider giving an input using ProvideInput().
            // Execution will not continue before an input is given.
            *requestInput = 1;
            if(nextChar) *nextChar = strippedCode[strippedIndex + 1];
            return ',';

        case '[':
            // Set the index to the stored index of the corresponding bracket.
            if(memory[memoryIndex] == 0)
                strippedIndex = *(uint16_t*)(strippedCode + strippedIndex + 1);
            strippedIndex += 2;
            break;

        case ']':
            // Set the index to the stored index of the corresponding bracket.
            if(memory[memoryIndex] != 0)
                strippedIndex = *(uint16_t*)(strippedCode + strippedIndex + 1);
            strippedIndex += 2;
            break;

        case '#':
            break;

        default:
            return INTERPRETER_LEAK;
        
    }
    strippedIndex++;
    if(strippedIndex == strippedLength) {
        return INTERPRETER_EOF;
    }

    if(nextChar) *nextChar = strippedCode[strippedIndex];
    return codeChar;
}

void ProvideInput(uint8_t input) {
    memory[memoryIndex] = input;
    strippedIndex++;
}


// The stripper strips code that looks like this:
// ```brainfuck
// +++ Set @0 to 3
// [- > +++ <] Set @1 to 3 * 3 = 9
// ```
// To this: (every character is a byte) ```
// Index: 0123456789abcd
// Code:  +++[II->+++]II```
//
// Here, II contain the index of the corresponding bracket, so it would be:
// `+++[0b->+++]03`
//
// TODO: Count the maximum depth of [loops] and malloc the stack using that number.
// It's a bit more elegant that way, and you can nest more than 256 times.
uint16_t StripCode(char* code) {
    uint16_t strippedSize = 0;
    char* c = code;

    // Go through the entire code to measure how long our stripped code array needs to be.
    while(*c) {
        if(strchr(KEY_CHARACTERS, *c)) {
            strippedSize++;
            // I do the bracket jumping by storing the corresponding bracket for each bracket in the code,
            // so we need to make space for that.
            if(*c == '[' || *c == ']') strippedSize += 2;
        }
        c++;
    }
    // Room for a terminating \0.
    strippedSize++;

    // Allocate the arrays.
    strippedCode = (char *) malloc(strippedSize * sizeof(char));
    strippedIndeces = (uint16_t *) malloc(strippedSize * sizeof(uint16_t));


    // Go through the code and actually strip it.
    uint16_t strippedIndex = 0;
    uint16_t codeIndex = 0;
    uint16_t bracketStack[MAX_LOOP_DEPTH];
    uint16_t *stackPtr = bracketStack;

    while(code[codeIndex]) {
        if(strchr(KEY_CHARACTERS, code[codeIndex])) {
            strippedCode[strippedIndex] = code[codeIndex];
            strippedIndeces[strippedIndex] = codeIndex;

            // Push the index onto the stack if it's an opening bracket.
            if(code[codeIndex] == '[') {
                // Push the index onto the stack.
                *stackPtr++ = strippedIndex;

                // Continue to next character.
                strippedIndex += 2;
            }


            // Pop an index off the stack if it's a closing bracket.
            else if(code[codeIndex] == ']') {
                stackPtr--;
                // Set the index of this bracket to the corresponding index from the stack.
                *(uint16_t*)(strippedCode + strippedIndex + 1) = *stackPtr;
                // Set the index of the opposite bracket to this index.
                *(uint16_t*)(strippedCode + *stackPtr + 1) = strippedIndex;

                // Continue to the next character.
                strippedIndex += 2;
            }

            strippedIndex++;
        }
        codeIndex++;
    }

    strippedCode[strippedIndex] = '\0';
    return strippedSize;
}



uint16_t GetCodeIndex(void) {
    return strippedIndeces[strippedIndex];
}

uint16_t GetMemIndex(void) {
    return memoryIndex;
}

uint8_t* GetMemory(void) {
    return memory;
}


// Thanks Michael on SO. I could write this myself but I truly cannot be bothered.
// https://stackoverflow.com/questions/2029103/correct-way-to-read-a-text-file-into-a-buffer-in-c
char *FileToBuffer(const char *fileName) {

    FILE *fp = fopen(fileName, "r");
    if(fp == NULL) return NULL;

    char* buffer = NULL;

    if (fp != NULL) {
        // Go to the end of the file.
        if (fseek(fp, 0L, SEEK_END) == 0) {
            // Get the size of the file.
            long bufsize = ftell(fp);
            if (bufsize == -1) return NULL;

            // Allocate our buffer to that size.
            buffer = malloc(sizeof(char) * (bufsize + 1));

            // Go back to the start of the file.
            if (fseek(fp, 0L, SEEK_SET) != 0) return NULL;

            // Read the entire file into memory.
            size_t newLen = fread(buffer, sizeof(char), bufsize, fp);
            if ( ferror( fp ) != 0 ) {
                fputs("Error reading file", stderr);
                return NULL;
            } else {
                (buffer)[newLen++] = '\0'; // Just to be safe.
            }
        }
        fclose(fp);
    }

    return buffer;
}

