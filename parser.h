#ifndef PARSERH
#define PARSERH

#define DEBUG 1		// our custom debugging

extern int yylex();
int yyerror(char *err);
extern int yylineno;
extern char *yytext;

// helper to indent to a specific depth
extern int indi;
#define INDENT(n)\
	for (indi = 0; indi < depth; indi++) \
		fprintf(stdout, "  ")

#endif