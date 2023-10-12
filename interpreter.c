#include <stdlib.h>


#include "interpreter.h"



char* fullCode;
char* strippedCode;
uint16_t* strippedCodeIndeces;

static char *FileToBuffer(const char *fileName);


char* InitInterpreter(const char* inFileName, void (*output)(char out)) {
    fullCode = FileToBuffer(inFileName);
    return fullCode;
}

char InterpretNextChar(void);

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