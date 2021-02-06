%debug
%{
#define YYDEBUG	1
#define parse.error verbose

#include "parser.h"

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

/* reference: https://en.cppreference.com/w/c/language/operator_precedence */
%left	','

%right	'=' PLUSEQ MINUSEQ TIMESEQ DIVEQ MODEQ SHLEQ SHREQ ANDEQ XOREQ OREQ

%left	'?' ':'		/* ternary */
%left	LOGOR
%left	LOGAND
%left	'|'
%left	'^'
%left	'&'		/* bitwise binary op */
%left	EQEQ NOTEQ
%left	'<' LTEQ '>' GTEQ
%left	SHL SHR
%left	'+' '-'		/* arithmetic binary ops */
%left	'*' '/' '%'	/* arithmetic binary ops */

/* this precedence level also includes unary +, -, *, &, post(inc|dec)rement,
   casting */
%left	'!' '~' SIZEOF

/* this precedence level also includes pre(inc|dec)rement */
%left	'(' ')' '[' ']' '.' INDSEL

%type <astnode>	expr
%type <ident> IDENT
%%
exprlist:	expr		{print_astnode($1);}

expr:	IDENT			{ALLOC($$);$$->ident=(struct astnode_ident)
				 {NT_IDENT,$1};}
	| NUMBER		{ALLOC($$);
				 $$->num=(struct astnode_number)
				 {0,(struct number){}};}
	| expr '+' expr		{ALLOC_SET_BINOP($$, '+', $1, $3);}
	| expr '-' expr		{ALLOC_SET_BINOP($$, '-', $1, $3);}
	| expr '*' expr		{ALLOC_SET_BINOP($$, '*', $1, $3);}
	| expr '/' expr		{/* TODO: check for division by zero if
				 $3 is a literal zero constant */
				 if (!$3) fprintf(stderr, "/0 err\n");
				 else ALLOC_SET_BINOP($$, '/', $1, $3);}
	| '+' expr %prec '!'    {ALLOC_SET_UNOP($$, '+', $2);}
	| '-' expr %prec '!'    {ALLOC_SET_UNOP($$, '-', $2);}
	| '*' expr %prec '!'    {ALLOC_SET_UNOP($$, '*', $2);}
	| '&' expr %prec '!'    {ALLOC_SET_UNOP($$, '&', $2);}
	| '(' expr ')'		{$$=$2;}
	;
%%
// TODO: do we have to cover cases of divide by zero symbolically?
// TODO: deal with pre/post increment
// TODO: deal with casting
// TODO: is ternary parsing correct?

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

char *astnodetype_tostring(enum astnode_type type)
{
	switch (type) {
		case NT_NUMBER:		return "NT_NUMBER";
		case NT_KEYWORD:	return "NT_KEYWORD";
		case NT_OPERATOR:	return "NT_OPERATOR";
		case NT_STRING:		return "NT_STRING";
		case NT_CHARLIT:	return "NT_CHARLIT";
		case NT_IDENT:		return "NT_IDENT";

		case NT_BINOP:		return "NT_BINOP";
		case NT_UNOP:		return "NT_UNOP";

		default:
			fprintf(stderr, "Error: unknown AST node type.\n");
			return "";
	}
}

void print_astnode_recursive(union astnode *node, int depth)
{
	INDENT(depth);
	fprintf(stdout, "AST node: %s\n",
		astnodetype_tostring(node->generic.type));

	switch (node->generic.type) {
		case NT_NUMBER:
			INDENT(depth);
			fprintf(stdout, "value: %d\n", node->num.num.int_val);
			break;
		case NT_BINOP:
			INDENT(depth);
			fprintf(stdout, "op: %c\n", node->binop.operator);
			print_astnode_recursive(node->binop.left, depth+1);
			print_astnode_recursive(node->binop.right, depth+1);
			break;
		case NT_UNOP:
			INDENT(depth);
			fprintf(stdout, "op: %c\n", node->unop.operator);
			print_astnode_recursive(node->unop.arg, depth+1);
			break;
		case NT_IDENT:
			INDENT(depth);
			fprintf(stdout, "ident: %s\n", node->ident.ident);
			break;
		default:
			fprintf(stdout, "unknown op\n");
	}
}

void print_astnode(union astnode *node)
{
	fprintf(stdout, "\n\n\nExpression AST:\n");
	print_astnode_recursive(node, 0);
	fprintf(stdout, "\n\n\n");
}