#ifndef VMWRITER_H
#define VMWRITER_H

typedef enum {
    CONST,
    ARG,
    LOCAL,
    THIS,
    THAT,
    POINTER,
    TEMP
} Segment;

typedef enum {
    ADD,
    SUB,
    NEG,
    EQ,
    GT,
    LT,
    AND,
    OR,
    NOT
} Operation;

char* segmentToString(Segment segment);
char* operationToString(Operation op);

void initOutput(char* inputFilename);
void closeOutput();
void writePush(Segment segment, char* index);
void writePop(Segment segment, char* index);
void writeArithmetic(Operation op);
void writeLabel(char* label);
void writeGoto(char* label);
void writeIf(char* label);
void writeCall(char* name, int nArgs);
void writeFunction(char* name, int nLocals);
void writeReturn();

#endif // VMWRITER_H