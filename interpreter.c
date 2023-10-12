#include <stdlib.h>
#include <string.h>


#include "interpreter.h"

#define KEY_CHARACTERS "#[]+-<>.,"

// I don't feel like doing this dynamically.
#define MAX_LOOP_DEPTH 256


char* fullCode;
char* strippedCode;
uint16_t* strippedIndeces;

static char *FileToBuffer(const char *fileName);
static uint16_t StripCode(char* code);


char* InitInterpreter(const char* inFileName, void (*output)(char out)) {
    fullCode = FileToBuffer(inFileName);
    StripCode(fullCode);
    return fullCode;
}

char InterpretNextChar(void);



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

    // Allocate the arrays.
    strippedCode = (char *) malloc(strippedSize * sizeof(char));
    strippedIndeces = (uint16_t *) malloc(strippedSize * sizeof(uint16_t));


    // Go through the code and actually strip it.
    uint16_t index = 0;
    while(*c) {
        if(strchr(KEY_CHARACTERS, *c)) {
            strippedCode[index] = *c;
            // TODO: Implement the stack and the bracket thing.
        }
        c++;
    }
    return strippedSize;
}



// Thanks Michael on SO. I could write this myself but I truly cannot be bothered.
// https://stackoverflow.com/questions/2029103/correct-way-to-read-a-text-file-into-a-buffer-in-c
char *FileToBuffer(const char *fileName) {

    FILE *fp = fopen(fileName, "r");
    if(fp == NULL) return NULL;

    char* buffer;

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

