
/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Symbol Tables Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Adam Crawley
Student ID: 201490505
Email: sc21ac@leeds.ac.uk
Date Work Commenced: 23/04/2023
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbols.h"

Symbol* head = NULL;
Symbol* tail = NULL;

int kStaticNumber = 0;
int kFieldNumber = 0;
int kArgumentNumber = 0;
int kVarNumber = 0;
int kClassNumber = 0;
int kSubroutineNumber = 0;

/** SCOPE REFERS TO A SYMBOL DEFINES IT
 * e.g.
 * class symbol (scope = null)
 * function/class attr symbol (scope = class symbol)
 * statement list symbol (scope = function symbol)
 */

void setKind(Symbol* symbol, Kind kind) {
    symbol->kind = kind;
}
void setType(Symbol* symbol, char type[128]) {
    strcpy(symbol->type, type);
}

// Returns status code
ParserInfo setIdentifier(Symbol* symbol, Token token) {
    ParserInfo pi;
    Symbol* existing = getSymbol(token.lx, symbol->scope);
    if (existing != NULL && existing->isPendingDeclaration) {
        existing->isPendingDeclaration = 0;
    } else if (existing != NULL && existing->scope == symbol->scope) {
        pi.er = redecIdentifier;
        pi.tk = token;
        return pi;
    }
    strcpy(symbol->identifier, token.lx);
    pi.er = none;
    return pi;
}
ParserInfo setIdentifierByString(Symbol* symbol, char identifier[128]) {
    ParserInfo pi;
    Symbol* existing = getSymbol(identifier, symbol->scope);
    if (existing != NULL && existing->isPendingDeclaration) {
        existing->isPendingDeclaration = 0;
    }
    strcpy(symbol->identifier, identifier);
    pi.er = none;
    return pi;
}

Symbol* declaredSymbol(Symbol* scope, Kind kind) {
    return insertSymbol(scope, kind, 0);
}

// Symbol* usedSymbol(Symbol* scope, char identifier[128]) {
//     // If we use a symbol that hasn't been declared then insert it
//     Symbol* obtained = getSymbol(identifier, scope);
//     if (obtained == NULL) {
//         return insertSymbol(scope, kind, 1);
//     }

//     // Establish redeclarations
//     obtained->isDeclaredTwice = 1;
//     return obtained;
// }


Symbol* usedExternalSymbol(Symbol* scope, char identifier1[128], char identifier2[128]) {
    // identifier1.identifier2
    // identifier1.identifer2[expression]
    // identifier1.identifier2(expressionList)

    // Check if local variable that is of userdefined type
    Symbol* existing = getSymbol(identifier1, scope);
    char className[128] = {0};
    if (existing != NULL) {
        strcpy(className, existing->type);
    } else {
        strcpy(className, identifier1);
    }

    // Now we have class/type name, check for attr existing in that class' scope, otherwise add it
    Symbol* classScope = getSymbol(className, NULL); // See if class exists
    if (classScope == NULL) {
        // Then class not yet defined, add pending declaration for class and attr
        classScope = insertSymbol(NULL, kClass, 1);
        setIdentifierByString(classScope, className);
        setType(classScope, "class");
    }
    Symbol* attrScope = getSymbol(identifier2, classScope); // Check if attribute exists
    if (attrScope == NULL) {
        attrScope = insertSymbol(classScope, kVar, 1);
        setIdentifierByString(attrScope, identifier2);
    }
    return attrScope;
}


/**
 * Inserts a new symbol in the given scope (when supplied with parent scope)
*/
Symbol* insertSymbol(Symbol* scope, Kind kind, char isPendingDeclaration) {
    // Assemble Symbol struct
    Symbol* s = malloc(sizeof(Symbol));
    s->isPendingDeclaration = isPendingDeclaration;
    s->next = NULL;
    s->scope = scope;

    // Set number attribute
    if (kind == kClass) s->number = kClassNumber++;
    else if (kind == kStatic) s->number = kStaticNumber++;
    else if (kind == kField) s->number = kFieldNumber++;
    else if (kind == kArgument) s->number = kArgumentNumber++;
    else if (kind == kVar) s->number = kVarNumber++;
    else if (kind == kSubroutine) s->number = kSubroutineNumber++;
    s->kind = kind;

    // Add to linked list
    if (tail == NULL) {
        head = s;
        tail = s;
    } else {
        tail->next = s;
        tail = s;
    }
    // Return Symbol
    return tail;
}

/**
 * Function to check whether a given symbol, is accessible in the current scope
*/
int canGetSymbol() {
    return 1;
}

/**
 * Gets a symbol from the symbol table using the current scope
 * Returns NULL if not found
*/
Symbol* getSymbol(char identifier[128], Symbol* scope) {
    // Search through all of current scope, return if found
    // Iteratively go to parent scope of current scope repeat process    
    Symbol* currentScope = scope;
    
    if (currentScope == NULL) { // Then we are looking for classes
        Symbol* curr = head;
        while (curr != NULL) {
            // check if correct symbol and in scope
            if (curr->scope == NULL && !strcmp(identifier, curr->identifier)) {
                return curr;
            }
            curr = curr->next;
        }
    }

    while (currentScope != NULL) {
        Symbol* curr = head;
        while (curr != NULL) {
            // check if correct symbol and in scope
            if (currentScope == curr->scope && !strcmp(identifier, curr->identifier)) {
                return curr;
            }
            curr = curr->next;
        }
        // change scope to parent scope
        currentScope = currentScope->scope;
    }
    return NULL;
}

/**
 * Frees up all the memory allocated to the symbol table
*/
void freeSymbolTable() {
    Symbol* curr = head;
    while (curr != NULL) {
        Symbol* next = curr->next;
        free(curr);
        curr = next;
    }
}
char* kindAsString(Kind kind) {
    switch (kind) {
        case kClass: return "kClass";
        case kStatic: return "kStatic";
        case kField: return "kField";
        case kArgument: return "kArgument";
        case kVar: return "kVar";
        case kSubroutine: return "kSubroutine";
        default: return "";
    }
} 
void printSymbolTable() {
    Symbol* curr = head;
    while (curr != NULL) {
        Symbol* next = curr->next;
        if (curr->kind == kClass) {
            printf("------------\n");
        }
        printSymbol(curr);
        curr = next;
    }
}
void printSymbol(Symbol* s) {
    printf("<%s, %s, %s, n%i, p%i, %p (%s)>\n", s->identifier, s->type, kindAsString(s->kind), s->number, s->isPendingDeclaration, s->scope, s->scope->identifier);
}