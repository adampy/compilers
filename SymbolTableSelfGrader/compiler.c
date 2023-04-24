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
#include "SymbolGrader.h"


int InitCompiler() {
	return 1;
}

ParserInfo compile(char* dir_name) {
	ParserInfo p;
	DIR *dir;
    struct dirent *de;

	// Find all jack files
    dir = opendir(dir_name);
    while(dir) {
        de = readdir(dir);
        if (!de || de->d_type != 8) break;

		// Check if jack file
		char* filename = de->d_name;
		char* extension = strrchr(filename, '.');
		if (extension == NULL) continue;
		if (strcmp(extension, ".jack")) continue;

		// Assemble full filename
		char fullFilename[128] = {0};
		strcpy(fullFilename, dir_name);
		strcat(fullFilename, "/");
		strcat(fullFilename, filename);

		// Init lexer and run parser
		//printf("--- Parsing %s ---\n", fullFilename);
		InitLexer(fullFilename);
		ParserInfo pi = Parse(); // Parser adds to global symbol table
		//PrintError(pi);
		//printf("Completed parsing!\n");
		if (pi.er != none) {
			return pi;
		}
    }
    closedir(dir);

	// Ensure scope is valid
	printSymbolTable();
	p.er = none;

	StopParser();
	return p;
}

int StopCompiler() {
	return 1;
}


#ifndef TEST_COMPILER
int main() {
	InitCompiler();
	ParserInfo p = compile("Pong");
	printf("--- Overall compilation error ---\n");
	PrintError(p);
	StopCompiler();
	return 1;
}
#endif
