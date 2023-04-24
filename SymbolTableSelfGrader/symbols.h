#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"
#include "parser.h"

// define your own types and function prototypes for the symbol table(s) module below

typedef enum {
    kStatic,
    kField,
    kArgument,
    kVar
} Kind;

typedef struct {
    char[128] identifier,
    char[128] type,
    Kind kind,
    Symbol* scope,
    Symbol* next
} Symbol;


#endif
