#ifndef PARSERH
#define PARSERH

#define DEBUG 1		// our custom debugging

extern int yylex();
int yyerror(char *err);
int yyerror_fatal(char *err);
extern int yylineno;
extern char *yytext;

#endif