#ifndef COMPILER_H
#define COMPILER_H

#define TEST_COMPILER    // uncomment to run the compiler autograder

#include "parser.h"
#include "symbols.h"
#include <stdlib.h>
#include <stdio.h>

void initOutput(char* inputFilename);
void closeOutput();
FILE* getOutput();

int InitCompiler ();
ParserInfo compile (char* dir_name);
int StopCompiler();

#endif
