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

// TODO: FIX EOF ERRORS
// TODO: Maybe things are being added to the buffer even though they shouldnt be?

FILE* jackFile;
FILE* file; // lexer buffer output file
char* jackFileName;
char prevRead;
char buffer[512];
int posInBuffer; // Stores the next character to read from the buffer
int charsReadIntoBuffer;
int isReadingMultiLineComment;
int isReadingComment;
int newLinesComsumed; // Number of newlines consumed by tokeniser + 1 (sinc we start at line 1)
int isBufferInitialised; // Stores whether the buffer has been filled inserted at least once

#define NUMBER_OF_RESWORDS 21
const char RESWORDS[NUMBER_OF_RESWORDS][12] = {
  "class",
  "constructor",
  "method",
  "function",
  "int",
  "boolean",
  "char",
  "void",
  "var",
  "static",
  "field",
  "let",
  "do",
  "if",
  "else",
  "while",
  "return",
  "true",
  "false",
  "null",
  "this"
};

/** Returns 1 if the given string is a reserved word, and 0 otherwise */
int isResword(char* lexeme) {
  for (int i = 0; i < NUMBER_OF_RESWORDS; i++) {
    if (strcmp(lexeme, RESWORDS[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

/** Consumes from the buffer and passes the string literal to output until a " is found
 * If newline is present, 2 is returned
 * If EOF present, 1 is returned
 * If successful, 0 is returned
 * `posInBuffer` is not incremented inside this function
 * */
int consumeStringLiteral(char output[128]) {
  int stringLength = 0;
  char c;
  char literalBuffer[128] = {'\0'};
  int tempPosInBuffer = posInBuffer + 1; // Move past the " into the literal
  while (1) {
    c = buffer[tempPosInBuffer];
    if (c == '"') break;
    else if (c == '\n') return 2;
    else if (c == EOF) return 1;
    tempPosInBuffer++;

    literalBuffer[stringLength] = c; // Need * since output is a pointer to char array
    literalBuffer[++stringLength] = '\0';
  }
  strcpy(output, literalBuffer);
  return 0;
}

/**
Discards `chars` characters from `buffer`
*/
void discardFromBuffer(int chars) {
  for (int i = posInBuffer; i < sizeof(buffer); i++) {
    //printf("buffer[%d] = buffer[%d]\n", i-chars, i + chars - posInBuffer);
    buffer[i - chars] = buffer[i + chars - posInBuffer];
  }
  
  charsReadIntoBuffer -= posInBuffer;
  posInBuffer = 0;
}

/** 
Fills `buffer` with characters until it is full OR a newline or EOF is found.
It also edits the `posInBuffer` index.

Comments are handled by this function, and will return 1 if an EOF is found
within a comment.

If an EOF is found in a comment or string, it returns the line number of the EOF
If the filling of the buffer is valid, then 0 is returned
*/
int totalCharsAddedToBuffer = 0;
int fillBuffer() {
  // Discard all prev elements if they exist
  if (posInBuffer != 0) {
    discardFromBuffer(posInBuffer);
  }

  // Read in `posInBuffer` new characters
  char next;
  int newLinesFound = 1;
  while (1) {
    next = fgetc(jackFile);
    //printf("<|%d %d|%c %c|%d %d|%d|%d %d|>\n", isReadingComment, isReadingMultiLineComment, prevRead, next, posInBuffer, charsReadIntoBuffer, next==EOF, newLinesFound, newLinesComsumed);
    // Check for EOF in multi-line comment
    if (isReadingMultiLineComment && next == EOF) return newLinesFound;

    // Skip over if we are commenting
    if (next == '\n') {
      newLinesFound++;
      if (isReadingComment) {
        isReadingComment = 0;
        buffer[posInBuffer + charsReadIntoBuffer++] = next;
        goto skip;
      } else if (isReadingMultiLineComment) {
        buffer[posInBuffer + charsReadIntoBuffer++] = next;
      }
    }

    if (isReadingMultiLineComment && prevRead == '*' && next == '/') {
      isReadingMultiLineComment = 0;
      //next = ' '; 
      buffer[posInBuffer + charsReadIntoBuffer++] = ' '; // Replace "a/*...*/b" with "a b" - these comments can delimit
      //charsReadIntoBuffer++;
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
    // if (totalCharsAddedToBuffer - 0 < 100) {
    //   printf("|%d|%c|%c|%d|%d|%d|%d|\n", totalCharsAddedToBuffer, next, prevRead, isReadingComment, isReadingMultiLineComment, posInBuffer, charsReadIntoBuffer);
    // }
    buffer[posInBuffer + charsReadIntoBuffer++] = next;
    totalCharsAddedToBuffer++;
    if (next == '\n') {
      goto skip;
    } else if (next == EOF) break; // Leave EOF in buffer

    skip:
      prevRead = next;
      if (posInBuffer + charsReadIntoBuffer >= sizeof(buffer)) break;
  }

  //fprintf(file, "%s\n-------------------|%d|%d|%c|%c|-----------------\n", buffer, isReadingComment, isReadingMultiLineComment, next, prevRead);

  posInBuffer = 0;
  return 0;
}

Token fillBufferReturnToken() {
  Token t;
  strcpy(t.fl, jackFileName);
  int returnCode = fillBuffer();
  if (returnCode > 0) {
    t.ln = returnCode;
    t.ec = EofInCom;
    t.tp = ERR;
    strcpy(t.lx, "Error: unexpected eof in comment");
  }
  return t;
}

char* tokenToStringMapping[7] = {
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
  memset(buffer, 0, 512);
  posInBuffer = 0;
  prevRead = '\0';
  isReadingMultiLineComment = 0;
  isReadingComment = 0;
  charsReadIntoBuffer = 0;
  newLinesComsumed = 1;
  isBufferInitialised = 0;

  jackFile = fopen(fileName, "r");
  jackFileName = fileName;
  if (jackFile == NULL) return 0;
  return 1;
}


// Get the next token from the source file
Token GetNextToken () {  
  Token t = PeekNextToken();
  if (t.tp != ERR) { // Then we can consume from the buffer
    if (t.tp == STRING) {
      posInBuffer += 2; // String contains x2 ", we need to consume those too
    }
    posInBuffer += strlen(t.lx); // Now consume `t.lx` from buffer if valid
  }
  fillBuffer();
  return t;
}

typedef enum {Letter, Digit, Other, Symbol, Whitespace, InvalidSymbol, Quotation} CharType;
CharType getCharType(char c) {
  CharType type;
  if (isdigit(c)) type = Digit;
  else if (isalpha(c) || c == 95) type = Letter; // Also allow underscores
  else if (isspace(c)) type = Whitespace;
  else if (
    (c == 38) ||
    (40 <= c && c <= 47) ||
    (59 <= c && c <= 62) ||
    (c == 91 || c == 93) ||
    (123 <= c && c <= 126)
  ) type = Symbol;
  else if (c == 34) type = Quotation;
  else if (c > 32) {
    type = InvalidSymbol;
  } else {
    type = Other;
  }
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
Token PeekNextToken() {
  // Set token memory
  Token t;
  memset(&t, 0, sizeof(Token));
  memset(t.fl, 0, 32*sizeof(char));
  memset(t.lx, 0, 128*sizeof(char));
  strcpy(t.fl, jackFileName);

  // Fill buffer if this is the first call
  if (isBufferInitialised == 0) {
    int returnCode = fillBuffer();
      if (returnCode > 0) {
      t.ln = returnCode;
      t.ec = EofInCom;
      t.tp = ERR;
      strcpy(t.lx, "Error: unexpected eof in comment");
      return t;
    }
    isBufferInitialised++;
  }

  //fprintf(file, "%s", buffer);

  // Consume whitespace
  char c;
  CharType type;
  while (1) {
    c = buffer[posInBuffer];
    // Check for EOF
    if (c == EOF) {
      t.tp = EOFile;
      strcpy(t.lx, "End of File");
      t.ln = newLinesComsumed;
      return t;
    }

    type = getCharType(c);
    // Check for InvalidSymbol or Quotation
    if (type == InvalidSymbol) {
      t.tp = ERR;
      t.ln = newLinesComsumed;
      strcpy(t.lx, "Error: illegal symbol in source file");
      t.ec = IllSym;
      return t;
    } else if (type == Quotation) {
      t.tp = STRING;
      t.ln = newLinesComsumed;
      int returnCode = consumeStringLiteral(t.lx);
      if (returnCode == 1) {
        // EOF in literal
        t.tp = ERR;
        t.ec = EofInStr;
        strcpy(t.lx, "Error: unexpected eof in string constant");
      } else if (returnCode == 2) {
        // Newline in literal
        t.tp = ERR;
        t.ec = EofInStr;
        strcpy(t.lx, "Error: new line in string constant");
      }
      return t;
    }

    if (type != Whitespace) break;
    if (c == '\n') newLinesComsumed++;
    posInBuffer++;
  }

  // Char found in `c` with `type`
  int tempPosInBuffer = posInBuffer + 1; // +1 since the pointers are on the next character to read
  char lexeme[128] = {c, '\0'};
  int lexemeLength = 1;
  
  // Continue consuming allowed types (if not symbol, since all symbols are 1 char)
  if (type != Symbol) {
    char nextChar;
    while (1) {
      nextChar = buffer[tempPosInBuffer];
      if (!validLexemeCharacter(type, nextChar)) break;
      lexeme[lexemeLength] = nextChar;
      lexeme[++lexemeLength] = '\0';
      tempPosInBuffer++; // Increment if successful, so if nextChar is different the char is not lost
    }
  }

  // Lexeme present in `lexeme`
  //printf("(%s) - %d\n", lexeme, type);
  strcpy(t.lx, lexeme);
  t.ln = newLinesComsumed;
  if (type == Letter) {
    if (isResword(lexeme)) {
      t.tp = RESWORD;
    } else {
      t.tp = ID;
    }
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
/*#ifndef TEST
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
  if (!InitLexer(argv[1])) { // Iff error
    printf("Error: Bad input filename\n");
    return 1;
  }

  file = fopen("lexerOutput.txt", "w");
  FILE* file2 = fopen("lexerTokenOutput.txt", "w");
  
  // Accounts for EOF in comment and string to begin wtih
  Token t = fillBufferReturnToken();
  if (t.tp == ERR) {
    writeTokenToFile(file2, &t);
    return 0;
  }

  while (1) {
    Token t = GetNextToken();
    writeTokenToFile(file2, &t);
    if (t.tp == EOFile || t.tp == ERR) {
      break;
    }
  }

  fclose(file);
  fclose(file2);
  StopLexer(0);
	return 0;
}
// do not remove the next line
#endif
*/