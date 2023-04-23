/************************************************************************
University of Leeds
School of Computing
COMP2932- Compiler Design and Construction
Lexer Module

I confirm that the following code has been developed and written by me and it is entirely the result of my own work.
I also confirm that I have not copied any parts of this program from another person or any other source or facilitated someone to copy this program from me.
I confirm that I will not publish the program online or share it with anyone without permission of the module leader.

Student Name: Adam Crawley
Student ID: 201490505
Email: sc21ac@leeds.ac.uk
Date Work Commenced: 11/02/2023
*************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"


// YOU CAN ADD YOUR OWN FUNCTIONS, DECLARATIONS AND VARIABLES HERE

FILE* jackFile;
char* jackFileName;
int isReachedEOF;
char prevRead;
int lineNumber;
char buffer[512];
int posInBuffer; // Stores the next character to read from the buffer
int isReadingMultiLineComment;
int isReadingComment;

/**
Discards `chars` characters from `buffer`
*/
void discardFromBuffer(int chars) {
  for (int i = posInBuffer; i < sizeof(buffer); i++) {
    buffer[i - chars] = buffer[chars];
  }
}

/** 
Fills `buffer` with characters until it is full OR a newline or EOF is found.
It also edits the `posInBuffer` index.

Comments are handled by this function, and will return 1 if an EOF is found
within a comment.
*/
int fillBuffer() {
  // Discard all prev elements if they exist
  if (prevRead != '\0') {
    discardFromBuffer(posInBuffer);
    posInBuffer = 0;
  }

  // Read in `posInBuffer` new characters
  int charsReadIntoBuffer = 0;
  while (1) {
    char next = fgetc(jackFile);
    //printf("%d %d | %d | %c %c | %d %d\n", isReadingComment, isReadingMultiLineComment, lineNumber, prevRead, next, posInBuffer, charsReadIntoBuffer);
    // Check for EOF in multi-line comment
    if (isReadingMultiLineComment && next == EOF) return 1;

    // Skip over if we are commenting
    if (next == '\n') {
      if (isReadingComment) {
        isReadingComment = 0;
        goto skip;
      } else if (isReadingMultiLineComment) {
        lineNumber++;
      }
    }

    if (isReadingMultiLineComment && prevRead == '*' && next == '/') {
      isReadingMultiLineComment = 0;
      goto skip;
    }

    // Check for new comments
    if (!isReadingMultiLineComment && !isReadingComment && prevRead == '/' && next == '/') {
      if (charsReadIntoBuffer > 0) {
        charsReadIntoBuffer--;
      }
      isReadingComment = 1;
    }
    if (!isReadingComment && prevRead == '/' && next == '*') {
      if (charsReadIntoBuffer > 0) {
        charsReadIntoBuffer--;
      }
      isReadingMultiLineComment = 1;
    }
    if (isReadingComment || isReadingMultiLineComment) {
      goto skip;
    }

    // At this point, we know we are not reading a comment
    buffer[posInBuffer + charsReadIntoBuffer++] = next;
    if (next == '\n') {
      lineNumber++;
      goto skip;
    } else if (next == EOF) {
      isReachedEOF = 1;
      buffer[posInBuffer + charsReadIntoBuffer - 1] = '\0'; // Replace EOF with null char
      break;
    }

    if (posInBuffer + charsReadIntoBuffer >= sizeof(buffer)) break;
    skip:
      prevRead = next;
  }

  posInBuffer = 0;
  return 0;
}

char tokenToStringMapping[7][7] = {
  "RESWORD",
  "ID",
  "INT",
  "SYMBOL",
  "STRING",
  "EOFile",
  "ERR"
};
void writeTokenToFile(FILE* file, Token* t) {
  fprintf(file, "< %s, %d, %s, %s >\n", t->fl, t->ln, t->lx, tokenToStringMapping[t->tp]);
}


// IMPLEMENT THE FOLLOWING functions
//***********************************

// Initialise the lexer to read from source file
// file_name is the name of the source file
// This requires opening the file and making any necessary initialisations of the lexer
// If an error occurs, the function should return 0
// if everything goes well the function should return 1
int InitLexer (char* fileName) {
  posInBuffer = 0;
  lineNumber = 1;
  isReachedEOF = 0;
  prevRead = '\0';
  isReadingMultiLineComment = 0;
  isReadingComment = 0;

  jackFile = fopen(fileName, "r");
  if (jackFile == NULL) return 1;
  return 0;
}


// Get the next token from the source file
Token GetNextToken () {
	/*Token t;
  t.tp = ERR;
  return t;*/
  
  Token t = PeekNextToken();
  // Now "consume" t from bufer
  posInBuffer += strlen(t.lx);
  return t;
}

typedef enum {Letter, Digit, Other, Symbol, Whitespace} CharType;
CharType getCharType(char c) {
  CharType type;
  if (48 <= c && c <= 57) type = Digit;
  else if ((65 <= c && c <= 90) || (97 <= c && c <= 122)) type = Letter;
  else if ((9 <= c && c <= 13) || c == 32) type = Whitespace;
  else if (
    (c == 38) ||
    (40 <= c && c <= 47) ||
    (59 <= c && c <= 62) ||
    (c == 91 || c == 93) ||
    (123 <= c && c <= 126)
  ) type = Symbol;
  else type = Other;
  return type;
}

int validLexemeCharacter(CharType type, char c) {
  CharType nextType = getCharType(c);
  if (type == nextType || (type == Letter && nextType == Digit)) {
    return 1;
  }
  return 0;
}

// peek (look) at the next token in the source file without removing it from the stream
// Moves `posInBuffer` when consuming initial whitespace
// Does not move `posInBuffer` when consuming a token
Token PeekNextToken () {
  Token t;
  t.tp = ERR;

  char c;
  CharType type;
  while (1) {
    c = buffer[posInBuffer];
    type = getCharType(c);
    if (type != Whitespace) break;
    posInBuffer++;
  }

  // Char found in `c` with `type`
  int tempPosInBuffer = posInBuffer + 1; // +1 since the pointers are on the next character to read
  char lexeme[128] = {c, '\0'};
  int lexemeLength = 1;
  
  // Continue consuming allowed types
  char nextChar;
  while (1) {
    nextChar = buffer[tempPosInBuffer];
    if (!validLexemeCharacter(type, nextChar)) break;
    lexeme[lexemeLength] = nextChar;
    lexeme[++lexemeLength] = '\0';
    tempPosInBuffer++; // Increment if successful, so if nextChar is different the char is not lost
  }

  // Lexeme present in `lexeme`
  //printf("(%s) - %d\n", lexeme, type);
  strcpy(t.lx, lexeme);
  strcpy(t.fl, jackFileName);
  t.ln = lineNumber;
  if (type == Letter) {
    t.tp = ID;
  } else if (type == Symbol) {
    t.tp = SYMBOL;
  } else if (type == Digit) {
    t.tp = INT;
  }

  return t;
}

// clean out at end, e.g. close files, free memory, ... etc
int StopLexer () {
  fclose(jackFile);
	return 0;
}

// do not remove the next line
#ifndef TEST
int main (int argc, char *argv[]) {
	// implement your main function here
  // NOTE: the autograder will not use your main function

  // Argument check
  if (argc == 1) {
    printf("Usage: ./lexer filename.jack\n");
    return 0;
  } else if (argc != 2) {
    printf("Error: Bad argument count\n");
    return 1;
  }

  // Continue with nominal execution
  if (InitLexer(argv[1])) {
    printf("Error: Bad input filename\n");
    return 1;
  }
  jackFileName = argv[1];

  FILE* file = fopen("lexerOutput.txt", "w");
  FILE* file2 = fopen("lexerTokenOutput.txt", "w");
  
  while (!isReachedEOF) {
    if (fillBuffer()) {
      Token t;
      t.tp = ERR;
      strcpy(t.lx, "Error: unexpected eof in comment");
      t.ec = EofInCom;
      t.ln = lineNumber;
      strcpy(t.fl, jackFileName);
      writeTokenToFile(file2, &t);
      break;
    }

    // Print nominally
    for (int i = 0; i < 20; i++) {
      //Token t = GetNextToken();
      //writeTokenToFile(file2, &t);
    }
    fprintf(file, "%s", buffer);
    isReachedEOF = 1;
  }

  fclose(file);
  fclose(file2);
  StopLexer(0);
	return 0;
}
// do not remove the next line
#endif
