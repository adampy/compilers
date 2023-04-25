#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lexer.h"
#include "parser.h"
#include "symbols.h"

// Modified get token and peek token
ParserInfo lexerErrorInfo;

/**
 * Wrapper to place any lexer errors when performing `GetNextToken`
 * inside a global `ParserInfo` object, called `lexerErrorInfo`
*/
Token mGetNextToken() {
	Token t = GetNextToken();
	if (t.tp == ERR) {
		lexerErrorInfo.er = lexerErr;
		lexerErrorInfo.tk = t;
	}
	return t;
}

// you can declare prototypes of parser functions below
ParserInfo statementList();
ParserInfo statementList(Symbol* scope);
ParserInfo expression();
ParserInfo expressionList();
ParserInfo expressionListWithScope(Symbol* scope);

// operand -> integerConstant | identifier [.identifier ] [ [ expression ] | (expressionList ) ] | (expression) | stringLiteral | true | false | null | this
ParserInfo operand() {
	ParserInfo pi;
	Token t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors

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
		t = mGetNextToken();
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
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
			mGetNextToken(); // Consume the .
			if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
			t = mGetNextToken(); // identifier
			if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
			if (t.tp != ID) {
				pi.er = idExpected;
				pi.tk = t;
				return pi;
			}
			t = PeekNextToken(); // Move onwards to [ or ( if we had .
		}

		// [ [ expression ] | ( expressionList ) ]
		if (t.tp == SYMBOL && !strcmp(t.lx, "[")) {
			mGetNextToken(); // Consume [
			if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
			pi = expression();
			if (pi.er != none) return pi;

			// ]
			t = mGetNextToken();
			if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
			if (t.tp != SYMBOL  || strcmp(t.lx, "]")) {
				pi.er = closeBracketExpected;
				pi.tk = t;
				return pi;
			}


		} else if (t.tp == SYMBOL && !strcmp(t.lx, "(")) {
			mGetNextToken(); // Consume (
			if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
			pi = expressionList();
			if (pi.er != none) return pi;

			// )
			t = mGetNextToken();
			if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
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
		mGetNextToken(); // Consume - or ~
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
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
		mGetNextToken(); // Consume * or /
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors

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
		mGetNextToken(); // Consume + or -
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors

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
		mGetNextToken(); // Consume =, >, or <
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors

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
		mGetNextToken(); // Consume & or |
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors

		// relationalExpression
		pi = relationalExpression();
		if (pi.er != none) return pi;
		t = PeekNextToken();
	}
	
	pi.er = none;
	return pi;
}

// returnStatemnt -> return [ expression ] ;
ParserInfo returnStatement(Symbol* scope) {
	ParserInfo pi;
	Token t;

	// return
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
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
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
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
		t = mGetNextToken();
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
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
ParserInfo expressionListWithScope(Symbol* scope) {
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
		t = mGetNextToken();
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
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
ParserInfo subroutineCallWithScope(Symbol* scope) {
	ParserInfo pi;
	Token t;

	// identifier
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}

	// [ . identifier ]
	t = mGetNextToken(); // t is either . or (
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp == SYMBOL && !strcmp(t.lx, ".")) {
		// identifier
		t = mGetNextToken();
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
		if (t.tp != ID) {
			pi.er = idExpected;
			pi.tk = t;
			return pi;
		}
		t = mGetNextToken(); // Move onwards to ( if we had .
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	}
	
	// (
	if (t.tp != SYMBOL  || strcmp(t.lx, "(")) {
		pi.er = openParenExpected;
		pi.tk = t;
		return pi;
	}

	// expressionList
	pi = expressionListWithScope(scope);
	if (pi.er != none) return pi;

	// )
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL  || strcmp(t.lx, ")")) {
		pi.er = closeParenExpected;
		pi.tk = t;
		return pi;
	}

	pi.er = none;
	return pi;
}

// doStatement -> do subroutineCall ;
ParserInfo doStatement(Symbol* scope) {
	ParserInfo pi;
	Token t;

	// do
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != RESWORD || strcmp(t.lx, "do")) {
		pi.er = syntaxError;
		pi.tk = t;
		return pi;
	}

	// subroutineCall
	pi = subroutineCallWithScope(scope);
	if (pi.er != none) return pi;

	// ;
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL || strcmp(t.lx, ";")) {
		pi.er = semicolonExpected;
		pi.tk = t;
		return pi;
	}

	pi.er = none;
	return pi;
}

// whileStatement -> while ( expression ) { {statement} }
ParserInfo whileStatement(Symbol* scope) {
	ParserInfo pi;
	Token t;

	// while
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != RESWORD || strcmp(t.lx, "while")) {
		pi.er = syntaxError;
		pi.tk = t;
		return pi;
	}

	// (
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL  || strcmp(t.lx, "(")) {
		pi.er = openParenExpected;
		pi.tk = t;
		return pi;
	}

	// expression
	pi = expression();
	if (pi.er != none) return pi;

	// )
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL  || strcmp(t.lx, ")")) {
		pi.er = closeParenExpected;
		pi.tk = t;
		return pi;
	}

	// { {statement} }
	pi = statementList(scope);
	if (pi.er != none) return pi;

	pi.er = none;
	return pi;
}

// ifStatement -> if ( expression ) { {statement} } [else { {statement} }]
ParserInfo ifStatement(Symbol* scope) {
	ParserInfo pi;
	Token t;

	// if
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != RESWORD || strcmp(t.lx, "if")) {
		pi.er = syntaxError;
		pi.tk = t;
		return pi;
	}

	// (
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL  || strcmp(t.lx, "(")) {
		pi.er = openParenExpected;
		pi.tk = t;
		return pi;
	}

	// expression
	pi = expression();
	if (pi.er != none) return pi;

	// )
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL  || strcmp(t.lx, ")")) {
		pi.er = closeParenExpected;
		pi.tk = t;
		return pi;
	}

	// { {statement} }
	pi = statementList(scope);
	if (pi.er != none) return pi;

	// [else { {statement} }]
	t = PeekNextToken();
	if (t.tp == RESWORD && !strcmp(t.lx, "else")) {
		mGetNextToken(); // Consume else token
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
		pi = statementList(scope);
		if (pi.er != none) return pi;
	}

	pi.er = none;
	return pi;
}

// letStatement -> let identifier [ [ expression ] ] = expression ;
ParserInfo letStatement(Symbol* scope) {
	ParserInfo pi;
	Token t;

	// let
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != RESWORD || strcmp(t.lx, "let")) {
		pi.er = syntaxError;
		pi.tk = t;
		return pi;
	}

	// identifier
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}
	Symbol* variable = getSymbol(t.lx, scope);
	if (variable == NULL) {
		pi.er = undecIdentifier;
		pi.tk = t;
		return pi;
	}

	// [ [expression] ]
	t = mGetNextToken(); // Here t can be = or a [
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp == SYMBOL && !strcmp(t.lx, "[")) {
		// expression
		pi = expression();
		if (pi.er != none) return pi;

		// ]
		t = mGetNextToken();
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
		if (t.tp != SYMBOL || strcmp(t.lx, "]")) {
			pi.er = closeBracketExpected;
			pi.tk = t;
			return pi;
		}

		t = mGetNextToken(); // Move onwards, such that is a =
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
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
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
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
	Token t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors

	if (!isType(t)) {
		pi.er = illegalType;
		pi.tk = t;
		return pi;
	}

	pi.er = none;
	return pi;
}
ParserInfo typeWithSymbol(Symbol* symbol) {
	ParserInfo pi;
	Token t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors

	if (!isType(t)) {
		pi.er = illegalType;
		pi.tk = t;
		return pi;
	}
	
	setType(symbol, t.lx);
	pi.er = none;
	return pi;
}

// varDeclarStatement -> var type identifier { , identifier } ;
ParserInfo varDeclarstatement(Symbol* scope) {
	ParserInfo pi;
	Token t;
	Symbol* firstVar = declaredSymbol(scope, kVar);

	// var
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != RESWORD || strcmp(t.lx, "var")) {
		pi.er = syntaxError;
		pi.tk = t;
		return pi;
	}

	// type
	pi = typeWithSymbol(firstVar);
	if (pi.er != none) return pi;

	// identifier
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}
	pi = setIdentifier(firstVar, t);

	// {, identifier}
	t = PeekNextToken();
	while ((t.tp != SYMBOL || strcmp(t.lx, ";"))) {
		// ,
		t = mGetNextToken();
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
		if (t.tp != SYMBOL  || strcmp(t.lx, ",")) {
			pi.er = syntaxError;
			pi.tk = t;
		}

		// identifier
		t = mGetNextToken();
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
		if (t.tp != ID) {
			pi.er = idExpected;
			pi.tk = t;
			return pi;
		}

		Symbol* var = declaredSymbol(scope, kVar);
		setType(var, firstVar->type);
		pi = setIdentifier(var, t);
		if (pi.er != none) return pi;

		t = PeekNextToken();
	}

	// ;
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL || strcmp(t.lx, ";")) {
		pi.er = semicolonExpected;
		pi.tk = t;
		return pi;
	}

	pi.er = none;
	return pi;
}

// statement -> varDeclarStatement | letStatement | ifStatement | whileStatement | doStatement | returnStatement
ParserInfo statement(Symbol* scope) {
	ParserInfo pi;
	Token t;

	t = PeekNextToken();
	if (t.tp != RESWORD) {
		pi.er = syntaxError;
		pi.tk = t;
		return pi;
	}

	if (!strcmp(t.lx, "var")) {
		pi = varDeclarstatement(scope);
	} else if (!strcmp(t.lx, "let")) {
		pi = letStatement(scope);
	} else if (!strcmp(t.lx, "if")) {
		pi = ifStatement(scope);
	} else if (!strcmp(t.lx, "while")) {
		pi = whileStatement(scope);
	} else if (!strcmp(t.lx, "do")) {
		pi = doStatement(scope);
	} else if (!strcmp(t.lx, "return")) {
		pi = returnStatement(scope);
	} else {
		pi.er = syntaxError; // Is a resword but not a statement
	}

	return pi;

	pi.er = none;
	return pi;
}

// statementList -> { {statement} }
ParserInfo statementList(Symbol* scope) {
	ParserInfo pi;
	Token t;

	// {
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL || strcmp(t.lx, "{")) {
		pi.er = openBraceExpected;
		pi.tk = t;
		return pi;
	}

	// {statement}
	t = PeekNextToken();
	while ((t.tp != SYMBOL || strcmp(t.lx, "}"))) {
		pi = statement(scope);
		if (pi.er != none) return pi;
		t = PeekNextToken();
	}

	// }
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL || strcmp(t.lx, "}")) {
		pi.er = closeBraceExpected;
		pi.tk = t;
		return pi;
	}

	pi.er = none;
	return pi;
}

// subroutineBody -> { {statement} }
ParserInfo subroutineBody(Symbol* subroutine) {
	return statementList(subroutine);
}

// paramList -> type identifier {, type identifier} | ε
ParserInfo paramList(Symbol* subroutine) {
	ParserInfo pi;
	Token t;
	
	// Early return if empty (i.e. no type present, therefore empty paramList)
	t = PeekNextToken();
	if (!isType(t)) {
		pi.er = none;
		return pi;
	}

	// type
	Symbol* firstParam = declaredSymbol(subroutine, kArgument);
	pi = typeWithSymbol(firstParam);
	if (pi.er != none) return pi;

	// identifier
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}
	setIdentifier(firstParam, t);

	// {, type identifier}
	t = PeekNextToken(); // t is either a comma or not
	while ((t.tp == SYMBOL && !strcmp(t.lx, ","))) { // Repeat whilst comma
		// ,
		t = mGetNextToken();
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
		if (t.tp != SYMBOL  || strcmp(t.lx, ",")) {
			pi.er = syntaxError;
			pi.tk = t;
		}

		// type
		Symbol* param = declaredSymbol(subroutine, kArgument);
		pi = typeWithSymbol(param);
		if (pi.er != none) return pi;

		// identifier
		t = mGetNextToken();
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
		if (t.tp != ID) {
			pi.er = idExpected;
			pi.tk = t;
			return pi;
		}
		setIdentifier(param, t);

		t = PeekNextToken();
	}

	pi.er = none;
	return pi;
}

// subroutineDeclar -> (constructor | function | method) (type|void) identifier (paramList) subroutineBody
ParserInfo subroutineDeclar(Symbol* class) {
	ParserInfo pi;
	Symbol* subroutine = declaredSymbol(class, kSubroutine);

	Token t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors

	// (constructor | function | method)
	if (t.tp != RESWORD || (strcmp(t.lx, "constructor") && strcmp(t.lx, "function") && strcmp(t.lx, "method"))) {
		pi.er = subroutineDeclarErr;
		pi.tk = t;
		return pi;
	}

	// (type|void)
	t = PeekNextToken();
	if (t.tp == RESWORD && !strcmp(t.lx, "void")) {
		mGetNextToken(); // Consume void
		setType(subroutine, "void");
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	} else {
		pi = typeWithSymbol(subroutine); //pi = type();
		if (pi.er != none) return pi;
	}

	// identifier
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}
	pi = setIdentifier(subroutine, t);
	if (pi.er != none) return pi;

	// (
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL  || strcmp(t.lx, "(")) {
		pi.er = openParenExpected;
		pi.tk = t;
		return pi;
	}

	// paramList
	pi = paramList(subroutine);
	if (pi.er != none) return pi;

	// )
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL  || strcmp(t.lx, ")")) {
		pi.er = closeParenExpected;
		pi.tk = t;
		return pi;
	}

	// subroutineBody
	pi = subroutineBody(subroutine);
	if (pi.er != none) return pi;
	
	pi.er = none;
	return pi;
}

// classVarDeclar -> (static | field) type identifier {, identifier} ;
ParserInfo classVarDeclar(Symbol* class) {
	ParserInfo pi;
	Symbol* classVar = declaredSymbol(class, kVar);

	// (static | field)
	Token t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != RESWORD || (strcmp(t.lx, "static") && strcmp(t.lx, "field"))) {
		pi.er = classVarErr;
		pi.tk = t;
		return pi;
	}

	if (!strcmp(t.lx, "static")) setKind(classVar, kStatic);
	else if (!strcmp(t.lx, "field")) setKind(classVar, kField);

	// type
	pi = typeWithSymbol(classVar);
	if (pi.er != none) return pi;

	// identifier
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}
	pi = setIdentifier(classVar, t);
	if (pi.er != none) return pi;
	
	// {, identifier}
	t = PeekNextToken();
	while ((t.tp != SYMBOL || strcmp(t.lx, ";"))) {
		// ,
		t = mGetNextToken();
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
		if (t.tp != SYMBOL  || strcmp(t.lx, ",")) {
			pi.er = syntaxError;
			pi.tk = t;
		}

		// identifier
		t = mGetNextToken();
		if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
		if (t.tp != ID) {
			pi.er = idExpected;
			pi.tk = t;
			return pi;
		}

		Symbol* newClassVar = declaredSymbol(class, kVar);
		setType(newClassVar, classVar->type);
		setKind(newClassVar, classVar->kind);
		pi = setIdentifier(newClassVar, t);
		if (pi.er != none) return pi;		

		t = PeekNextToken();
	}

	// ;
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo; // Check for lexer errors
	if (t.tp != SYMBOL || strcmp(t.lx, ";")) {
		pi.er = semicolonExpected;
		pi.tk = t;
		return pi;
	}

	pi.er = none;
	return pi;
}

// memberDeclar -> classVarDeclar | subroutineDeclar
ParserInfo memberDeclar(Symbol* class) {
	Token t = PeekNextToken();
	if (t.tp == RESWORD && (!strcmp(t.lx, "static") || !strcmp(t.lx, "field"))) {
		return classVarDeclar(class);
	} else if (t.tp == RESWORD && (!strcmp(t.lx, "constructor") || !strcmp(t.lx, "function") || !strcmp(t.lx, "method"))) {
		return subroutineDeclar(class);
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
	Token t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo;

	if (t.tp != RESWORD || strcmp(t.lx, "class")) {
		pi.er = classExpected;
		pi.tk = t;
		return pi;
	}

	// identifier
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo;
	if (t.tp != ID) {
		pi.er = idExpected;
		pi.tk = t;
		return pi;
	}
	Symbol* class = declaredSymbol(NULL, kClass);
	pi = setIdentifier(class, t);
	if (pi.er != none) return pi;
	setType(class, "class");

	// {
	t = mGetNextToken();
	if (t.tp != SYMBOL || strcmp(t.lx, "{")) {
		pi.er = openBraceExpected;
		pi.tk = t;
		return pi;
	}

	// {memberDeclar}
	t = PeekNextToken();
	while ((t.tp != SYMBOL || strcmp(t.lx, "}"))) {
		pi = memberDeclar(class);
		if (pi.er != none) return pi;
		t = PeekNextToken();
	}

	// }
	t = mGetNextToken();
	if (lexerErrorInfo.er != none) return lexerErrorInfo;
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
	freeSymbolTable();
	return 1;
}

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