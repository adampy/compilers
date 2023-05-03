#include <stdio.h>
#include <string.h>
#include "vmwriter.h"

FILE* output = NULL;

void initOutput(char* inputFilename) {
    char outputFilename[128] = {0};
    strcpy(outputFilename, inputFilename); // Move filename to output name
    char* extension = strstr(outputFilename, ".jack");
    strcpy(extension, ".vm"); // Replace .jack with .vm
    output = fopen(outputFilename, "w");
}

void closeOutput() {
    fclose(output);
    output = NULL;
}

void writeReturn() {
    fprintf(output, "return\n");
}