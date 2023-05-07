#include <stdio.h>
#include <string.h>
#include "vmwriter.h"

FILE* output = NULL;

// ----- FILE HANDLING FUNCTIONS -----

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

// ----- STRING FUNCTIONS -----

char* segmentToString(Segment segment) {
    switch (segment) {
        case CONST: return "constant";
        case ARG: return "argument";
        case LOCAL: return "local";
        case THIS: return "this";
        case THAT: return "that";
        case POINTER: return "pointer";
        case TEMP: return "temp";
        case STATIC: return "static";
        default: return "";
    }
}

char* operationToString(Operation op) {
    switch (op) {
        case ADD: return "add";
        case SUB: return "sub";
        case NEG: return "neg";
        case EQ: return "eq";
        case GT: return "gt";
        case LT: return "lt";
        case AND: return "and";
        case OR: return "or";
        case NOT: return "not";
        default: return "";
    }
} 

// ----- WRITING FUNCTIONS -----

void writePush(Segment segment, char* index) {
    if (output == NULL) return;
    fprintf(output, "push %s %s\n", segmentToString(segment), index);
}

void writePop(Segment segment, char* index)  {
    if (output == NULL) return;
    fprintf(output, "pop %s %s\n", segmentToString(segment), index);
}

void writeArithmetic(Operation op) {
    if (output == NULL) return;
    fprintf(output, "%s\n", operationToString(op));
}

void writeLabel(char* label) {
    if (output == NULL) return;
    fprintf(output, "label %s\n", label);
}

void writeGoto(char* label) {
    if (output == NULL) return;
    fprintf(output, "goto %s\n", label);
}

void writeIf(char* label) {
    if (output == NULL) return;
    fprintf(output, "if-goto %s\n", label);
}

void writeCall(char* name, int nArgs) {
    if (output == NULL) return;
    fprintf(output, "call %s %i\n", name, nArgs);
}

void writeFunction(char* name, int nLocals) {
    if (output == NULL) return;
    fprintf(output, "function %s %i\n", name, nLocals);
}

void writeReturn() {
    if (output == NULL) return;
    fprintf(output, "return\n");
}