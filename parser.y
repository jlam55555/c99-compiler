%debug
%{
#define YYDEBUG	1
#define parse.error verbose

#include "parser.h"

// shorthand for allocating a new AST node
#define ALLOC(var)\
	var = astnode_alloc();

%}
%union {
	struct number num;
	char *ident;
	struct string string;
	struct charlit charlit;
	union astnode *astnode;
}

%token	IDENT CHARLIT STRING NUMBER INDSEL PLUSPLUS MINUSMINUS SHL
%token	SHR LTEQ GTEQ EQEQ NOTEQ LOGAND LOGOR ELLIPSIS TIMESEQ DIVEQ
%token	MODEQ PLUSEQ MINUSEQ SHLEQ SHREQ ANDEQ OREQ XOREQ AUTO BREAK
%token	CASE CHAR CONST CONTINUE DEFAULT DO DOUBLE ELSE ENUM EXTERN
%token	FLOAT FOR GOTO IF INLINE INT LONG REGISTER RESTRICT RETURN
%token	SHORT SIGNED SIZEOF STATIC STRUCT SWITCH TYPEDEF UNION
%token	UNSIGNED VOID VOLATILE WHILE _BOOL _COMPLEX _IMAGINARY

%token 	    	NL
%left		'+' '-'
%left		'*' '/'
%nonassoc	'('

%type <astnode>	expr NUMBER
%%
exprlist:	expr			{print_astnode($1);}

expr:		NUMBER			{ALLOC($$);$$->num=(struct astnode_number){0,(struct number){}};}
		| expr '+' expr		{ALLOC($$);$$->binop=(struct astnode_binop){.type=NT_BINOP,.operator='+',.left=$1,.right=$3};}
		| expr '-' expr		{ALLOC($$);$$->binop=(struct astnode_binop){.type=NT_BINOP,.operator='-',.left=$1,.right=$3};}
		| expr '*' expr		{ALLOC($$);$$->binop=(struct astnode_binop){.type=NT_BINOP,.operator='*',.left=$1,.right=$3};}
		| expr '/' expr		{ALLOC($$);if (!$3) fprintf(stderr, "/0 err\n"); else $$->binop=(struct astnode_binop){.type=NT_BINOP,.operator='/',.left=$1,.right=$3};}
		| '-' expr %prec '('    {ALLOC($$);$$->unop=(struct astnode_unop){.type=NT_UNOP,.operator='-',.arg=$2};}
		| '(' expr ')'		{$$=$2;}
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

union astnode *astnode_alloc()
{
	return malloc(sizeof(union astnode));
}

void print_astnode_recursive(union astnode *node, int depth) {
	int i;

	for (i = 0; i < depth; i++) {
		fprintf(stdout, " ");
	}

	fprintf(stdout, "AST node: %d\n", node->generic.type);

	switch (node->generic.type) {
		case NT_NUMBER:
			fprintf(stdout, "value: %d\n", node->num.num.int_val);
			break;
		case NT_BINOP:
			print_astnode_recursive(node->binop.left, depth+1);
			print_astnode_recursive(node->binop.right, depth+1);
			break;
		case NT_UNOP:
			print_astnode_recursive(node->unop.arg, depth+1);
			break;
		default:
			fprintf(stdout, "unknown op\n");
	}
}

void print_astnode(union astnode *node)
{
	fprintf(stdout, "\n\n\nReading a node:\n");
	print_astnode_recursive(node, 0);
	fprintf(stdout, "\n\n\n");
}

/*
exprlist:   	expr			{print_astnode($1);}
        	| exprlist NL expr	{printf("EXPR VALUE IS  %d\n", $3);}
        	;
		*/