%debug
%{
#define YYDEBUG	1
#define parse.error verbose

// #include "tokensmanual.h"
// #define YYTOKENTYPE enum tokens;
#include "numutils.h"
#include "stringutils.h"

extern int yylex();
int yyerror(char *err);

struct astnode_binop {
	int nodetype;
	int operator;
	union astnode *left, *right;
};

struct astnode_number {
	int nodetype;
	struct number num;
};

union astnode {
	struct astnode_binop binop;
	struct astnode_number numop;
};

// struct astnode {
// 	int type;
// 	union astnodes {
// 		struct astnode_binop binop;
// 	} node;
// };
%}
%union {
	struct number num;
	char *ident;
	struct string string;
	struct charlit charlit;
	union astnode *astnode;
}

%token	IDENT
%token	CHARLIT
%token	STRING
%token	NUMBER
%token	INDSEL
%token	PLUSPLUS
%token	MINUSMINUS
%token	SHL
%token	SHR
%token	LTEQ
%token	GTEQ
%token	EQEQ
%token	NOTEQ
%token	LOGAND
%token	LOGOR
%token	ELLIPSIS
%token	TIMESEQ
%token	DIVEQ
%token	MODEQ
%token	PLUSEQ
%token	MINUSEQ
%token	SHLEQ
%token	SHREQ
%token	ANDEQ
%token	OREQ
%token	XOREQ
%token	AUTO
%token	BREAK
%token	CASE
%token	CHAR
%token	CONST
%token	CONTINUE
%token	DEFAULT
%token	DO
%token	DOUBLE
%token	ELSE
%token	ENUM
%token	EXTERN
%token	FLOAT
%token	FOR
%token	GOTO
%token	IF
%token	INLINE
%token	INT
%token	LONG
%token	REGISTER
%token	RESTRICT
%token	RETURN
%token	SHORT
%token	SIGNED
%token	SIZEOF
%token	STATIC
%token	STRUCT
%token	SWITCH
%token	TYPEDEF
%token	UNION
%token	UNSIGNED
%token	VOID
%token	VOLATILE
%token	WHILE
%token	_BOOL
%token	_COMPLEX
%token _IMAGINARY

// %token 		NUMBER
%token 	    NL
%left		'+' '-'
%left		'*' '/'
%nonassoc	'('

%type <astnode>	expr NUMBER
%%
exprlist:   expr    {printf("EXPR VALUE IS %d\n", $1);}
        | exprlist NL expr  {printf("EXPR VALUE IS  %d\n", $3);}
        ;
expr:		NUMBER			{$$->numop=(struct astnode_number){0,(struct number){}};}
		| expr '+' expr		{$$->numop=(struct astnode_number){1,(struct number){}};/*$1+$3;*/}
		| expr '-' expr		{$$->numop=(struct astnode_number){2,(struct number){}};/*$$1-$3;*/}
		| expr '*' expr		{$$->numop=(struct astnode_number){3,(struct number){}};/*$1*$3;*/}
		| '-' expr %prec '('    {$$->numop=(struct astnode_number){4,(struct number){}};/*-1*$2;*/}
		| '(' expr ')'		{$$->numop=(struct astnode_number){5,(struct number){}};/*$2;*/}
		;
%%
int main()
{
	yydebug = 1;
	yyparse();
}

int yyerror(char *err)
{
	fprintf(stderr, "Syntax err: %s\n", err);
}

/*
		| expr '/' expr		{if (!$3) fprintf(stderr, "/0 err\n"); else $$.val=$1/$2;}
		*/