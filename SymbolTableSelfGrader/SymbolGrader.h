#include "lexer.h"
#include "parser.h"

char* TokenTypeString (TokenType t);
char* ErrorString (SyntaxErrors e);
void PrintError (ParserInfo pn);
void ShowInfo (ParserInfo pn);
void PrintToken (Token t);