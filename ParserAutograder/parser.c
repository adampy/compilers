#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"


// you can declare prototypes of parser functions below

ParserInfo operand() {}
ParserInfo factor() {}
ParserInfo term() {}
ParserInfo arithmeticExpression() {}
ParserInfo relationalExpression() {}
ParserInfo expression() {}
ParserInfo returnStatement() {}
ParserInfo expressionList() {}
ParserInfo subroutineCall() {}
ParserInfo doStatement() {}
ParserInfo whileStatement() {}
ParserInfo ifStatement() {}
ParserInfo letStatement() {}
ParserInfo varDeclarStatement() {}
ParserInfo statement() {}
ParserInfo subroutineBody() {}
ParserInfo paramList() {}
ParserInfo subroutineDeclar() {}
ParserInfo type() {}
ParserInfo classVarDeclar() {}
ParserInfo memberDeclar() {
	ParserInfo pi;
	pi.er = none;
	return pi;
}

// classDeclar -> class identifier { {memberDeclar} }
ParserInfo classDeclar() {
	ParserInfo pi;

	// class
	Token t = GetNextToken();
	if (t.tp != RESWORD || strcmp(t.lx, "class")) {
		pi.er = classExpected;
		pi.tk = t;
		return pi;
	}

	// identifier {
	t = GetNextToken();
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}
	t = GetNextToken();
	if (t.tp != SYMBOL || strcmp(t.lx, "{")) {
		pi.er = openBraceExpected;
		pi.tk = t;
		return pi;
	}

	// {member Declar} }
	t = PeekNextToken();
	while ((t.tp != SYMBOL || strcmp(t.lx, "}"))) {
		pi = memberDeclar();
		if (pi.er != none)
			return pi;

		t = PeekNextToken();
	}
	t = GetNextToken();
	if (t.tp != SYMBOL || strcmp(t.lx, "}")) {
		pi.er = closeBraceExpected;
		pi.tk = t;
		return pi;
	}

	pi.er = none;
	return pi;
}


int InitParser (char* file_name) {
	return 1;
}

ParserInfo Parse () {
	ParserInfo pi = classDeclar();
	return pi;
}

int StopParser () {
	return 1;
}

#ifndef TEST_PARSER
int main (int argc, char *argv[]) {
	// Get filename argument
	if (argc != 2) {
		printf("Usage: ./parser fileToParse.jack");
		return 1;
	}
	InitParser();

	return 0;
}
#endif
