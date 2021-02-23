#include <stdio.h>
#include "errorutils.h"
#include "numutils.h"
#include "stringutils.h"
#include "parser.tab.h"

int lineno;
char filename[255] = "<stdin>";

extern char *yytext;

void parse_lineno() {
	char *c = yytext;
	int i = 0;

	// discard extra characters
	while (*c < '0' || *c > '9')
		++c;

	// read line number
	lineno = 0;
	while (*c >= '0' && *c <= '9')
		lineno = lineno * 10 + (*c++ - '0');

	// discard extra characters
	while (*c != '"')
		++c;
	++c;

	// read filename
	while (*c != '"')
		filename[i++] = *c++;
	filename[i] = 0;
}

void print_lexical_error() {
	fprintf(stderr, "%s: %d: Error: unexpected '%s'.\n",
		filename, lineno, yytext);
}

char *toktostr(int enumval) {
	switch (enumval) {
		case IDENT:	return "IDENT";
		case CHARLIT:	return "CHARLIT";
		case STRING:	return "STRING";
		case NUMBER:	return "NUMBER";
		case INDSEL:	return "INDSEL";
		case PLUSPLUS:	return "PLUSPLUS";
		case MINUSMINUS:return "MINUSMINUS";
		case SHL:	return "SHL";
		case SHR:	return "SHR";
		case LTEQ:	return "LTEQ";
		case GTEQ:	return "GTEQ";
		case EQEQ:	return "EQEQ";
		case NOTEQ:	return "NOTEQ";
		case LOGAND:	return "LOGAND";
		case LOGOR:	return "LOGOR";
		case ELLIPSIS:	return "ELLIPSIS";
		case TIMESEQ:	return "TIMESEQ";
		case DIVEQ:	return "DIVEQ";
		case MODEQ:	return "MODEQ";
		case PLUSEQ:	return "PLUSEQ";
		case MINUSEQ:	return "MINUSEQ";
		case SHLEQ:	return "SHLEQ";
		case SHREQ:	return "SHREQ";
		case ANDEQ:	return "ANDEQ";
		case OREQ:	return "OREQ";
		case XOREQ:	return "XOREQ";
		case AUTO:	return "AUTO";
		case BREAK:	return "BREAK";
		case CASE:	return "CASE";
		case CHAR:	return "CHAR";
		case CONST:	return "CONST";
		case CONTINUE:	return "CONTINUE";
		case DEFAULT:	return "DEFAULT";
		case DO:	return "DO";
		case DOUBLE:	return "DOUBLE";
		case ELSE:	return "ELSE";
		case ENUM:	return "ENUM";
		case EXTERN:	return "EXTERN";
		case FLOAT:	return "FLOAT";
		case FOR:	return "FOR";
		case GOTO:	return "GOTO";
		case IF:	return "IF";
		case INLINE:	return "INLINE";
		case INT: 	return "INT";
		case LONG:	return "LONG";
		case REGISTER:	return "REGISTER";
		case RESTRICT:	return "RESTRICT";
		case RETURN:	return "RETURN";
		case SHORT:	return "SHORT";
		case SIGNED:	return "SIGNED";
		case SIZEOF:	return "SIZEOF";
		case STATIC:	return "STATIC";
		case STRUCT:	return "STRUCT";
		case SWITCH:	return "SWITCH";
		case TYPEDEF:	return "TYPEDEF";
		case UNION:	return "UNION";
		case UNSIGNED:	return "UNSIGNED";
		case VOID:	return "VOID";
		case VOLATILE:	return "VOLATILE";
		case WHILE:	return "WHILE";
		case _BOOL:	return "_BOOL";
		case _COMPLEX:	return "_COMPLEX";
		case _IMAGINARY:return "_IMAGINARY";
	}
	return NULL;
}