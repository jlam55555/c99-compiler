#ifndef PARSERH
#define PARSERH

extern int yylex();
int yyerror(char *err);

// helper to indent to a specific depth
extern int indi;
#define INDENT(n)\
	for (indi = 0; indi < depth; indi++) \
		fprintf(stdout, "  ")

#endif