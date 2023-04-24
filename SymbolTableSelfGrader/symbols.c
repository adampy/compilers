
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
#include "symbols.h"

Symbol* head = NULL;
Symbol* tail = NULL;

/** SCOPE REFERS TO A SYMBOL DEFINES IT
 * e.g.
 * class symbol (scope = null)
 * function/class attr symbol (scope = class symbol)
 * statement list symbol (scope = function symbol)

/**
 * Inserts a new symbol in the given scope (when supplied with parent scope)
*/
Symbol* insertSymbol(Symbol* scope, Token token, Kind kind, char isPendingDeclaration) {
    // Assemble Symbol struct
    Symbol* s = malloc(sizeof(Symbol));
    s->token = token;
    s->kind = kind;
    s->isPendingDeclaration = isPendingDeclaration;
    s->next = NULL;

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
Symbol* getSymbol(Token token, Symbol* scope) {
    // Search through all of current scope, return if found
    // Iteratively go to parent scope of current scope repeat process
    Symbol* currentScope = scope;
    while (currentScope != NULL) {
        Symbol* curr = head;
        while (curr != NULL) {
            // check if correct symbol and in scope
            if (currentScope == curr->scope && token.lx == curr->token.lx) {
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

void printSymbolTable() {
    Symbol* curr = head;
    while (curr != NULL) {
        Symbol* next = curr->next;
        printSymbol(curr);
        curr = next;
    }
}
void printSymbol(Symbol* s) {
    printf("<%s, %i>\n", s->token.lx, s->kind);
}