#ifndef SYMBOLS_H
#define SYMBOLS_H

#include "lexer.h"
#include "parser.h"

// define your own types and function prototypes for the symbol table(s) module below

typedef enum {
    kStatic,
    kField,
    kArgument,
    kVar,
    kClass
} Kind;

typedef struct SymbolT {
    Token token; // The lexer token for this symbol
    char type[128]; // e.g. "class", "int", "BankAccount"
    int size; // How much storage do we need (in bytes)
    Kind kind; // What memory area we store this in
    int number; // The index of the kind
    struct SymbolT* scope; // Parent scope
    struct SymbolT* next; // The next linked list item
    char isPendingDeclaration; // Has the identifier been declared
} Symbol;


#endif
