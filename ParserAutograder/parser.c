#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"


// you can declare prototypes of parser functions below
ParserInfo statementList();
ParserInfo expression();
ParserInfo expressionList();

// operand -> integerConstant | identifier [.identifier ] [ [ expression ] | (expressionList ) ] | (expression) | stringLiteral | true | false | null | this
ParserInfo operand() {
	ParserInfo pi;
	Token t = GetNextToken();

	// Cases: true | false | null | this
	if (t.tp == RESWORD && (!strcmp(t.lx, "true") || !strcmp(t.lx, "false") || !strcmp(t.lx, "null") || !strcmp(t.lx, "this"))) {
		pi.er = none;
		return pi;
	}

	// Cases: integerConstant | stringLiteral
	if (t.tp == INT || t.tp == STRING) {
		pi.er = none;
		return pi;
	}

	// Cases: (expression) 
	if (t.tp == SYMBOL && !strcmp(t.lx, "(")) {
		// expression
		pi = expression();
		if (pi.er != none) return pi;

		// )
		t = GetNextToken();
		if (t.tp != SYMBOL  || strcmp(t.lx, ")")) {
			pi.er = closeParenExpected;
			pi.tk = t;
			return pi;
		}

		pi.er = none;
		return pi;
	}

	// Cases: identifier [ . identifier ] [ [ expression ] | ( expressionList ) ]
	if (t.tp == ID) {
		t = PeekNextToken(); // t is either . [ or ( 

		// [ . identifier ]
		if (t.tp == SYMBOL && !strcmp(t.lx, ".")) {
			// . identifier
			GetNextToken(); // Consume the .
			t = GetNextToken(); // identifier
			if (t.tp != ID) {
				pi.er = idExpected;
				pi.tk = t;
				return pi;
			}
			t = PeekNextToken(); // Move onwards to [ or ( if we had .
		}

		// [ [ expression ] | ( expressionList ) ]
		if (t.tp == SYMBOL && !strcmp(t.lx, "[")) {
			GetNextToken(); // Consume [
			pi = expression();
			if (pi.er != none) return pi;

			// ]
			t = GetNextToken();
			if (t.tp != SYMBOL  || strcmp(t.lx, "]")) {
				pi.er = closeBracketExpected;
				pi.tk = t;
				return pi;
			}


		} else if (t.tp == SYMBOL && !strcmp(t.lx, "(")) {
			GetNextToken(); // Consume (
			pi = expressionList();
			if (pi.er != none) return pi;

			// )
			t = GetNextToken();
			if (t.tp != SYMBOL || strcmp(t.lx, ")")) {
				pi.er = closeParenExpected;
				pi.tk = t;
				return pi;
			}
		}
		pi.er = none;
		return pi;
	}

	// Otherwise invalid operand - raise syntax error
	pi.er = syntaxError;
	pi.tk = t;
	return pi;
}

// factor -> ( - | ~ | ε ) operand
ParserInfo factor() {
	ParserInfo pi;
	Token t;

	// ( - | ~ | ε )
	t = PeekNextToken();
	if (t.tp == SYMBOL && (!strcmp(t.lx, "-") || !strcmp(t.lx, "~"))) {
		GetNextToken(); // Consume - or ~
	}

	// operand
	pi = operand();
	return pi;
}

// term -> factor { ( * | / ) factor }
ParserInfo term() {
	ParserInfo pi;
	Token t;

	// factor
	pi = factor();
	if (pi.er != none) return pi;

	// { ( * | / ) factor }
	t = PeekNextToken();
	while (t.tp == SYMBOL && (!strcmp(t.lx, "*") || !strcmp(t.lx, "/"))) {
		GetNextToken(); // Consume * or /

		// factor
		pi = factor();
		if (pi.er != none) return pi;
		t = PeekNextToken();
	}

	pi.er = none;
	return pi;
}

// arithmeticExpression -> term { ( + | - ) term }
ParserInfo arithmeticExpression() {
	ParserInfo pi;
	Token t;

	// term
	pi = term();
	if (pi.er != none) return pi;

	// { ( + | - ) term }
	t = PeekNextToken();
	while (t.tp == SYMBOL && (!strcmp(t.lx, "+") || !strcmp(t.lx, "-"))) {
		GetNextToken(); // Consume + or -

		// term
		pi = term();
		if (pi.er != none) return pi;
		t = PeekNextToken();
	}

	pi.er = none;
	return pi;
}

// relationalExpression -> arithmeticExpression { ( = | > | < ) arithmeticExpression }
ParserInfo relationalExpression() {
	ParserInfo pi;
	Token t;

	// arithmeticExpression
	pi = arithmeticExpression();
	if (pi.er != none) return pi;

	// { ( = | > | < ) arithmeticExpression }
	t = PeekNextToken();
	while (t.tp == SYMBOL && (!strcmp(t.lx, "=") || !strcmp(t.lx, ">") || !strcmp(t.lx, "<"))) {
		GetNextToken(); // Consume =, >, or <

		// arithmeticExpression
		pi = arithmeticExpression();
		if (pi.er != none) return pi;
		t = PeekNextToken();
	}

	pi.er = none;
	return pi;
}

int isPossibleExpression(Token t) {
	return
		(t.tp == STRING) ||
		(t.tp == INT) ||
		(t.tp == ID) ||
		(t.tp == RESWORD && !strcmp(t.lx, "true")) || 
		(t.tp == RESWORD && !strcmp(t.lx, "false")) || 
		(t.tp == RESWORD && !strcmp(t.lx, "null")) || 
		(t.tp == RESWORD && !strcmp(t.lx, "this")) || 
		(t.tp == SYMBOL && !strcmp(t.lx, "(")) || 
		(t.tp == SYMBOL && !strcmp(t.lx, "-")) || 
		(t.tp == SYMBOL && !strcmp(t.lx, "~"));
}
// expression -> relationalExpression { ( & | | ) relationalExpression }
ParserInfo expression() {
	ParserInfo pi;
	Token t;

	// relationalExpression
	pi = relationalExpression();
	if (pi.er != none) return pi;

	// { ( & | | ) relationalExpression }
	t = PeekNextToken();
	while (t.tp == SYMBOL && (!strcmp(t.lx, "&") || !strcmp(t.lx, "|"))) {
		GetNextToken(); // Consume & or |

		// relationalExpression
		pi = relationalExpression();
		if (pi.er != none) return pi;
		t = PeekNextToken();
	}
	
	pi.er = none;
	return pi;
}

// returnStatemnt -> return [ expression ] ;
ParserInfo returnStatement() {
	ParserInfo pi;
	Token t;

	// return
	t = GetNextToken();
	if (t.tp != RESWORD || strcmp(t.lx, "return")) {
		pi.er = syntaxError;
		pi.tk = t;
		return pi;
	}

	// [ expression ] ;
	t = PeekNextToken();
	if (isPossibleExpression(t)) {
		pi = expression();
		if (pi.er != none) return pi;
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

// expressionList -> expression { , expression } | ε
ParserInfo expressionList() {
	ParserInfo pi;
	Token t;

	// Early return if no expressions in list
	t = PeekNextToken();
	if (!isPossibleExpression(t)) {
		pi.er = none;
		return pi;
	}

	// expression
	pi = expression(); // At this point, we assume we have an expression in t
	if (pi.er != none) return pi;

	// {, expression}
	t = PeekNextToken(); // t is either a comma or not
	while ((t.tp == SYMBOL && !strcmp(t.lx, ","))) { // Repeat whilst comma
		// ,
		t = GetNextToken();
		if (t.tp != SYMBOL  || strcmp(t.lx, ",")) {
			pi.er = syntaxError;
			pi.tk = t;
			return pi;
		}

		// expression
		pi = expression();
		if (pi.er != none) return pi;
		t = PeekNextToken();
	}

	pi.er = none;
	return pi;
}

// subroutineCall -> identifier [ . identifier ] ( expressionList )
ParserInfo subroutineCall() {
	ParserInfo pi;
	Token t;

	// identifier
	t = GetNextToken();
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}

	// [ . identifier ]
	t = GetNextToken(); // t is either . or (
	if (t.tp == SYMBOL && !strcmp(t.lx, ".")) {
		// identifier
		t = GetNextToken();
		if (t.tp != ID) {
			pi.er = idExpected;
			pi.tk = t;
			return pi;
		}
		t = GetNextToken(); // Move onwards to ( if we had .
	}
	
	// (
	if (t.tp != SYMBOL  || strcmp(t.lx, "(")) {
		pi.er = openParenExpected;
		pi.tk = t;
		return pi;
	}

	// expressionList
	pi = expressionList();
	if (pi.er != none) return pi;

	// )
	t = GetNextToken();
	if (t.tp != SYMBOL  || strcmp(t.lx, ")")) {
		pi.er = closeParenExpected;
		pi.tk = t;
		return pi;
	}

	pi.er = none;
	return pi;
}

// doStatement -> do subroutineCall ;
ParserInfo doStatement() {
	ParserInfo pi;
	Token t;

	// do
	t = GetNextToken();
	if (t.tp != RESWORD || strcmp(t.lx, "do")) {
		pi.er = syntaxError;
		pi.tk = t;
		return pi;
	}

	// subroutineCall
	pi = subroutineCall();
	if (pi.er != none) return pi;

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

// whileStatement -> while ( expression ) { {statement} }
ParserInfo whileStatement() {
	ParserInfo pi;
	Token t;

	// while
	t = GetNextToken();
	if (t.tp != RESWORD || strcmp(t.lx, "while")) {
		pi.er = syntaxError;
		pi.tk = t;
		return pi;
	}

	// (
	t = GetNextToken();
	if (t.tp != SYMBOL  || strcmp(t.lx, "(")) {
		pi.er = openParenExpected;
		pi.tk = t;
		return pi;
	}

	// expression
	pi = expression();
	if (pi.er != none) return pi;

	// )
	t = GetNextToken();
	if (t.tp != SYMBOL  || strcmp(t.lx, ")")) {
		pi.er = closeParenExpected;
		pi.tk = t;
		return pi;
	}

	// { {statement} }
	pi = statementList();
	if (pi.er != none) return pi;

	pi.er = none;
	return pi;
}

// ifStatement -> if ( expression ) { {statement} } [else { {statement} }]
ParserInfo ifStatement() {
	ParserInfo pi;
	Token t;

	// if
	t = GetNextToken();
	if (t.tp != RESWORD || strcmp(t.lx, "if")) {
		pi.er = syntaxError;
		pi.tk = t;
		return pi;
	}

	// (
	t = GetNextToken();
	if (t.tp != SYMBOL  || strcmp(t.lx, "(")) {
		pi.er = openParenExpected;
		pi.tk = t;
		return pi;
	}

	// expression
	pi = expression();
	if (pi.er != none) return pi;

	// )
	t = GetNextToken();
	if (t.tp != SYMBOL  || strcmp(t.lx, ")")) {
		pi.er = closeParenExpected;
		pi.tk = t;
		return pi;
	}

	// { {statement} }
	pi = statementList();
	if (pi.er != none) return pi;

	// [else { {statement} }]
	t = PeekNextToken();
	if (t.tp == RESWORD && !strcmp(t.lx, "else")) {
		GetNextToken(); // Consume else token
		pi = statementList();
		if (pi.er != none) return pi;
	}

	pi.er = none;
	return pi;
}

// letStatement -> let identifier [ [ expression ] ] = expression ;
ParserInfo letStatement() {
	ParserInfo pi;
	Token t;

	// let
	t = GetNextToken();
	if (t.tp != RESWORD || strcmp(t.lx, "let")) {
		pi.er = syntaxError;
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

	// [ [expression] ]
	t = GetNextToken(); // Here t can be = or a [
	if (t.tp == SYMBOL && !strcmp(t.lx, "[")) {
		// expression
		pi = expression();
		if (pi.er != none) return pi;

		// ]
		t = GetNextToken();
		if (t.tp != SYMBOL || strcmp(t.lx, "]")) {
			pi.er = closeBracketExpected;
			pi.tk = t;
			return pi;
		}

		t = GetNextToken(); // Move onwards, such that is a =
	}
	
	// =
	if (t.tp != SYMBOL || strcmp(t.lx, "=")) {
		pi.er = equalExpected;
		pi.tk = t;
		return pi;
	}

	// expression
	pi = expression();
	if (pi.er != none) return pi;

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

int isType(Token t) {
	return (t.tp == RESWORD && (!strcmp(t.lx, "int") || !strcmp(t.lx, "char") || !strcmp(t.lx, "boolean"))) || t.tp == ID;
}
// type -> int | char | boolean | identifier
ParserInfo type() {
	ParserInfo pi;
	Token t = GetNextToken();

	if (!isType(t)) {
		pi.er = illegalType;
		pi.tk = t;
		return pi;
	}

	pi.er = none;
	return pi;
}

// varDeclarStatement -> var type identifier { , identifier } ;
ParserInfo varDeclarStatement() {
	ParserInfo pi;
	Token t;

	// var
	t = GetNextToken();
	if (t.tp != RESWORD || strcmp(t.lx, "var")) {
		pi.er = syntaxError;
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

		t = PeekNextToken();
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

// statement -> varDeclarStatement | letStatement | ifStatement | whileStatement | doStatement | returnStatement
ParserInfo statement() {
	ParserInfo pi;
	Token t;

	t = PeekNextToken();
	if (t.tp != RESWORD) {
		pi.er = syntaxError;
		pi.tk = t;
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

// statementList -> { {statement} }
ParserInfo statementList() {
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

// subroutineBody -> { {statement} }
ParserInfo subroutineBody() {
	return statementList();
}

// paramList -> type identifier {, type identifier} | ε
ParserInfo paramList() {
	ParserInfo pi;
	Token t;
	
	// Early return if empty (i.e. no type present, therefore empty paramList)
	t = PeekNextToken();
	if (!isType(t)) {
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

	// {, type identifier}
	t = PeekNextToken(); // t is either a comma or not
	while ((t.tp == SYMBOL && !strcmp(t.lx, ","))) { // Repeat whilst comma
		// ,
		t = GetNextToken();
		if (t.tp != SYMBOL  || strcmp(t.lx, ",")) {
			pi.er = syntaxError;
			pi.tk = t;
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
		return pi;
	}

	// paramList
	pi = paramList();
	if (pi.er != none) return pi;

	// )
	t = GetNextToken();
	if (t.tp != SYMBOL  || strcmp(t.lx, ")")) {
		pi.er = closeParenExpected;
		pi.tk = t;
		return pi;
	}

	// subroutineBody
	pi = subroutineBody();
	if (pi.er != none) return pi;
	
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

		t = PeekNextToken();
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

// char* ErrorString (SyntaxErrors e)
// {

// 	switch (e)
// 	{
// 		case none: return "no errors";
// 		case lexerErr: return "lexer error";
// 		case classExpected: return "keyword class expected";
// 		case idExpected: return "identifier expected";
// 		case openBraceExpected:	return "{ expected";
// 		case closeBraceExpected: return "} expected";
// 		case memberDeclarErr: return "class member declaration must begin with static, field, constructor , function , or method";
// 		case classVarErr: return "class variables must begin with field or static";
// 		case illegalType: return "a type must be int, char, boolean, or identifier";
// 		case semicolonExpected: return "; expected";
// 		case subroutineDeclarErr: return "subrouting declaration must begin with constructor, function, or method";
// 		case openParenExpected: return "( expected";
// 		case closeParenExpected: return ") expected";
// 		case closeBracketExpected: return "] expected";
// 		case equalExpected: return "= expected";
// 		case syntaxError: return "syntax error";
// 		default: return "not a valid error code";
// 	}
// }

// void ShowInfo (ParserInfo pn)
// {
// 	if (pn.er == none)
// 		printf ("none\n");
// 	else
// 		printf ("error type: %s, line: %i,token: %s, \n" , ErrorString(pn.er),  pn.tk.ln , pn.tk.lx  );
// }

#ifndef TEST_PARSER
int main (int argc, char *argv[]) {
	// Get filename argument
	if (argc != 2) {
		printf("Usage: ./parser fileToParse.jack\n");
		return 1;
	}
	InitLexer (argv[1]);
	ParserInfo p = Parse();
	if (p.er != none) {
		ShowInfo(p);
	}

	return 0;
}
#endif