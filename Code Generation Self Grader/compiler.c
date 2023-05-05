/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
The Compiler Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Adam Crawley
Student ID: 201490505
Email: sc21ac@leeds.ac.uk
Date Work Commenced: 23/04/2023
*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include "compiler.h"
#include "vmwriter.h"
//#include "SymbolGrader.h"

#define NUM_STANDARD_LIBS 8
char* standardLibraries[NUM_STANDARD_LIBS] = {
	"Math.jack",
	"Memory.jack",
	"Screen.jack",
	"Output.jack",
	"Keyboard.jack",
	"String.jack",
	"Array.jack",
	"Sys.jack"
};

int InitCompiler() {
	InitSymbols();
	return 1;
}

ParserInfo compile(char* dir_name) {
	ParserInfo p;
	p.er = none;
	DIR *dir = NULL;
    struct dirent *de = NULL;

	// Find all jack files
    dir = opendir(dir_name);
    while(dir) {
        de = readdir(dir);
        if (!de) break;

		// Check if jack file
		char* filename = de->d_name;
		if (!strcmp(filename, ".") || !strcmp(filename, "..")) continue;

		char* extension = strrchr(filename, '.');
		if (extension == NULL) continue;
		if (strcmp(extension, ".jack")) continue;

		// Assemble full filename
		char fullFilename[128] = {0};
		strcpy(fullFilename, dir_name);
		strcat(fullFilename, "/");
		strcat(fullFilename, filename);

		// Init lexer and run parser
		//initOutput(fullFilename);
		InitLexer(fullFilename);
		p = Parse(); // Parser adds to global symbol table
		if (p.er != none) {
			//PrintError(pi);
			break;
		}
		//closeOutput();
    }
    closedir(dir);

	if (p.er != none) {
		//printf("ere");
		return p;
	}

	// Import standard libraries
	for (int i = 0; i < NUM_STANDARD_LIBS; i++) {
		InitLexer(standardLibraries[i]);
		p = Parse();
		if (p.er != none) {
			return p;
		}
	}

	// If there are any pending declarations at this point then raise undefined
	Symbol* pending = getFirstPendingDeclaration();
	if (pending != NULL) {
		// printf("----------\n");
		// printSymbol(pending);
		// PrintToken(pending->token);
		p.er = undecIdentifier;
		p.tk = pending->token;
		return p;
	}

	// Sort out kType numbering
	completeNumbering();

	// Lex and parse again but with code generation enabled
	dir = opendir(dir_name);
    while(dir) {
        de = readdir(dir);
        if (!de) break;

		// Check if jack file
		char* filename = de->d_name;
		if (!strcmp(filename, ".") || !strcmp(filename, "..")) continue;

		char* extension = strrchr(filename, '.');
		if (extension == NULL) continue;
		if (strcmp(extension, ".jack")) continue;

		// Assemble full filename
		char fullFilename[128] = {0};
		strcpy(fullFilename, dir_name);
		strcat(fullFilename, "/");
		strcat(fullFilename, filename);

		// Init lexer and run parser
		setCodeGen(1);
		initOutput(fullFilename);
		InitLexer(fullFilename);
		p = Parse(); // Parser adds to global symbol table
		if (p.er != none) {
			//PrintError(pi);
			break;
		}
		closeOutput();
    }
    closedir(dir);

	if (p.er != none) {
		//printf("ere");
		return p;
	}

	// Import standard libraries
	for (int i = 0; i < NUM_STANDARD_LIBS; i++) {
		InitLexer(standardLibraries[i]);
		p = Parse();
		if (p.er != none) {
			return p;
		}
	}

	return p;
}

int StopCompiler() {
	StopParser();
	return 1;
}


#ifndef TEST_COMPILER
int main() {
	InitCompiler();
	ParserInfo p = compile("Fraction");
	//printSymbolTable();
	//printf("--- Overall compilation error ---\n");
	//PrintError(p);
	StopCompiler();
	return (p.er != none);
}
#endif
