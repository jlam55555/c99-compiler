/**
 * 	Header for functions defined in the parser.y file.
 *
 * 	Include this in other files to use the yyerror() and yyerror_fatal()
 * 	error printing functions.
 */

#ifndef PARSERH
#define PARSERH

#define DEBUG 1		// our custom debugging

extern int yylex();
int yyerror(char *err);
int yyerror_fatal(char *err);
extern int yylineno;
extern char *yytext;

#endif