/**
 * 	Header for functions defined in the parser.y file.
 *
 * 	Include this in other files to use the yyerror() and yyerror_fatal()
 * 	error printing functions.
 */

#ifndef PARSERH
#define PARSERH

extern int yylex();
int yyerror(const char *err);
int yyerror_fatal(const char *err);
extern int yylineno, yydebug;
extern char *yytext;

#endif