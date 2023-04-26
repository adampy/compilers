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
    kClass,
    kSubroutine
} Kind;

typedef struct SymbolT {
    Token token;
    char identifier[128]; // The lexer token for this symbol
    char type[128]; // e.g. "class", "int", "BankAccount"
    int size; // How much storage do we need (in bytes)
    Kind kind; // What memory area we store this in
    int number; // The index of the kind
    struct SymbolT* scope; // Parent scope
    struct SymbolT* next; // The next linked list item

    char isPendingDeclaration; // Has the identifier been declared
    char isDeclaredTwice; // Has the identifier been declared more than once
} Symbol;


void setKind(Symbol* symbol, Kind kind);
void setType(Symbol* symbol, char type[128]);
void setToken(Symbol* symbol, Token token);
ParserInfo setIdentifier(Symbol* symbol, Token token);
ParserInfo setIdentifierByString(Symbol* symbol, char identifier[128]);

Symbol* declaredSymbol(Symbol* scope, Kind kind);
// Symbol* usedSymbol(Symbol* scope, char identifier[128]);
Symbol* usedIdentifier(Symbol* scope, char identifier[128]);
Symbol* usedExternalSymbol(Symbol* scope, Token t1, Token t2);

/**
 * Inserts a new symbol in the given scope (when supplied with parent scope)
*/
Symbol* insertSymbol(Symbol* scope, Kind kind, char isPendingDeclaration);

/**
 * Function to check whether a given symbol, is accessible in the current scope
*/
int canGetSymbol();

Symbol* getFirstPendingDeclaration();

/**
 * Gets a symbol from the symbol table using the current scope
 * Returns NULL if not found
*/
Symbol* getSymbol(char identifier[128], Symbol* scope);

/**
 * Frees up all the memory allocated to the symbol table
*/
void freeSymbolTable();

void printSymbolTable();
void printSymbol(Symbol* s);

void InitSymbols();

#endif
