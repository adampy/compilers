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

// statement -> varDeclarStatement | letStatement | ifStatement | whileStatement | doStatement | returnStatement
ParserInfo statement() {
	ParserInfo pi;
	Token t;

	t = PeekNextToken();
	if (t.tp != RESWORD) {
		pi.er = syntaxError;
		pi.t = t;
		return pi;
	}

	if (!strcmp(t.lx, "var")) {
		pi = varDeclarStatement();
	} else if (!strcmp(t.lx, "let")) {
		pi = letStatement();
	} else if (!strcmp(t.lx, "if")) {
		pi = ifStatement();
	} else if (!strcmp(t.lx, "while")) {
		pi = whileStatement();
	} else if (!strcmp(t.lx, "do")) {
		pi = doStatement();
	} else if (!strcmp(t.lx, "return")) {
		pi = returnStatement();
	} else {
		pi.er = syntaxError; // Is a resword but not a statement
	}
	
	return pi;

	pi.er = none;
	return pi;
}

// subroutineBody -> { {statement} }
ParserInfo subroutineBody() {
	ParserInfo pi;
	Token t;

	// {
	t = GetNextToken();
	if (t.tp != SYMBOL || strcmp(t.lx, "{")) {
		pi.er = openBraceExpected;
		pi.tk = t;
		return pi;
	}

	// {statement}
	t = PeekNextToken();
	while ((t.tp != SYMBOL || strcmp(t.lx, "}"))) {
		pi = statement();
		if (pi.er != none) return pi;
		t = PeekNextToken();
	}

	// }
	t = GetNextToken();
	if (t.tp != SYMBOL || strcmp(t.lx, "}")) {
		pi.er = closeBraceExpected;
		pi.tk = t;
		return pi;
	}

	pi.er = none;
	return pi;
}

// paramList -> type identifier {, type identifier} | ε
ParserInfo paramList() {
	ParserInfo pi;
	Token t;
	
	// Early return if empty (i.e. no type present, therefore empty paramList)
	t = PeekNextToken();
	if (t.tp == RESWORD && ((strcmp(t.lx, "int") && strcmp(t.lx, "char") && strcmp(t.lx, "boolean")) ||
		t.tp != ID)) {
			pi.er = none;
			return pi;
		}

	// type
	pi = type();
	if (pi.er != none) return pi;

	// identifier
	t = GetNextToken();
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}

	// {, identifier}
	t = PeekNextToken();
	while ((t.tp == SYMBOL && !strcmp(t.lx, ","))) { // Repeat whilst comma
		// ,
		t = GetNextToken();
		if (t.tp != SYMBOL  || strcmp(t.lx, ",")) {
			pi.er = syntaxError;
			pi.tk = t;
		}

		// identifier
		t = GetNextToken();
		if (t.tp != ID) {
			pi.er = idExpected;
			pi.tk = t;
			return pi;
		}
		t = PeekNextToken();
	}

	pi.er = none;
	return pi;
}

// subroutineDeclar -> (constructor | function | method) (type|void) identifier (paramList) subroutineBody
ParserInfo subroutineDeclar() {
	ParserInfo pi;
	Token t = GetNextToken();

	// (constructor | function | method)
	if (t.tp != RESWORD || (strcmp(t.lx, "constructor") && strcmp(t.lx, "function") && strcmp(t.lx, "method"))) {
		pi.er = subroutineDeclarErr;
		pi.tk = t;
		return pi;
	}

	// (type|void)
	t = PeekNextToken();
	if (t.tp == RESWORD && !strcmp(t.lx, "void")) {
		GetNextToken(); // Consume void
	} else {
		pi = type();
		if (pi.er != none) return pi;
	}

	// identifier
	t = GetNextToken();
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}

	// (
	t = GetNextToken();
	if (t.tp != SYMBOL  || strcmp(t.lx, "(")) {
		pi.er = openParenExpected;
		pi.tk = t;
	}

	// paramList
	pi = paramList();
	if (pi.er != none) return pi;

	// )
	t = GetNextToken();
	if (t.tp != SYMBOL  || strcmp(t.lx, ")")) {
		pi.er = closeParenExpected;
		pi.tk = t;
	}

	// subroutineBody
	pi = subroutineBody();
	if (pi.er != none) return pi;
	
	pi.er = none;
	return pi;
}

// type -> int | char | boolean | identifier
ParserInfo type() {
	ParserInfo pi;
	Token t = GetNextToken();

	if (t.tp == RESWORD && ((strcmp(t.lx, "int") && strcmp(t.lx, "char") && strcmp(t.lx, "boolean")) ||
		t.tp != ID)) {
			pi.er = illegalType;
			pi.tk = t;
			return pi;
	}
	pi.er = none;
	return pi;
}

// classVarDeclar -> (static | field) type identifier {, identifier} ;
ParserInfo classVarDeclar() {
	ParserInfo pi;

	// (static | field)
	Token t = GetNextToken();
	if (t.tp != RESWORD || (strcmp(t.lx, "static") && strcmp(t.lx, "field"))) {
		pi.er = classVarErr;
		pi.tk = t;
		return pi;
	}

	// type
	pi = type();
	if (pi.er != none) return pi;

	// identifier
	t = GetNextToken();
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}
	
	// {, identifier}
	t = PeekNextToken();
	while ((t.tp != SYMBOL || strcmp(t.lx, ";"))) {
		// ,
		t = GetNextToken();
		if (t.tp != SYMBOL  || strcmp(t.lx, ",")) {
			pi.er = syntaxError;
			pi.tk = t;
		}

		// identifier
		t = GetNextToken();
		if (t.tp != ID) {
			pi.er = idExpected;
			pi.tk = t;
			return pi;
		}
	}

	// ;
	t = GetNextToken();
	if (t.tp != SYMBOL || strcmp(t.lx, ";")) {
		pi.er = semicolonExpected;
		pi.tk = t;
		return pi;
	}

	pi.er = none;
	return pi;
}

// memberDeclar -> classVarDeclar | subroutineDeclar
ParserInfo memberDeclar() {
	Token t = PeekNextToken();
	if (t.tp == RESWORD && (!strcmp(t.lx, "static") || !strcmp(t.lx, "field"))) {
		return classVarDeclar();
	} else if (t.tp == RESWORD && (!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || !strcmp(t.lx, "method"))) {
		return subroutineDeclar();
	} else {
		ParserInfo pi;
		pi.er = memberDeclarErr;
		pi.tk = t;
		return pi;
	}
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

	// identifier
	t = GetNextToken();
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}

	// {
	t = GetNextToken();
	if (t.tp != SYMBOL || strcmp(t.lx, "{")) {
		pi.er = openBraceExpected;
		pi.tk = t;
		return pi;
	}

	// {memberDeclar}
	t = PeekNextToken();
	while ((t.tp != SYMBOL || strcmp(t.lx, "}"))) {
		pi = memberDeclar();
		if (pi.er != none) return pi;
		t = PeekNextToken();
	}

	// }
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
