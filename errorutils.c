#include <stdio.h>
#include "errorutils.h"

int lineno;
char filename[255];

extern char *yytext;

void parse_lineno() {
	char *c = yytext;
	int i = 0;

	// discard extra characters
	while (*c < '0' || *c > '9')
		++c;

	// read lineno
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
	fprintf(stderr, "%s: %d:\nError: unexpected '%s'.\n",
		filename, lineno, yytext);
}