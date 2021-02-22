%debug
%{
#define YYDEBUG	1
#define parse.error verbose

#include "parser.h"

%}
%union {
	int sc;	// single-character tokens
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
%left<sc>	','

%right<sc>	'=' PLUSEQ MINUSEQ TIMESEQ DIVEQ MODEQ SHLEQ SHREQ ANDEQ XOREQ OREQ

%right<sc>	'?' ':'		/* ternary */
%left<sc>	LOGOR
%left<sc>	LOGAND
%left<sc>	'|'
%left<sc>	'^'
%left<sc>	'&'		/* bitwise binary op */
%left<sc>	EQEQ NOTEQ
%left<sc>	'<' LTEQ '>' GTEQ
%left<sc>	SHL SHR
%left<sc>	'+' '-'		/* arithmetic binary ops */
%left<sc>	'*' '/' '%'	/* arithmetic binary ops */

/* this precedence level also includes unary +, -, *, &, post(inc|dec)rement,
   casting */
%left<sc>	'!' '~' SIZEOF MINUSMINUS PLUSPLUS

/* this precedence level also includes pre(inc|dec)rement */
%left<sc>	'(' ')' '[' ']' '.' INDSEL

/* to correctly parse nested if...else statements */
%left IF
%left ELSE

%type<sc> uop
%type <astnode>	constant pexpr pofexpr arglist arglistopt uexpr
%type <astnode>	castexpr multexpr addexpr shftexpr relexpr eqexpr andexpr
%type <astnode>	xorexpr orexpr logandexpr logorexpr condexpr asnmtexpr expr
%type <ident> IDENT
%type <string> STRING
%type <charlit> CHARLIT
%type <num> NUMBER
%%
exprlist:	expr		{print_astnode($1);}

/* 6.4.4.3 */
 /*enumconst:	IDENT		{TODO: identifier declared as an
				 enum constant; might cause errors later by
				 conflicting with IDENT? } */
		

/* 6.4.4 */
constant:	NUMBER		{ALLOC($$);$$->num=(struct astnode_number){NT_NUMBER,NULL,NULL,$1};}
		/*| enumconst	{TODO}*/
		| CHARLIT	{ALLOC($$);$$->charlit=(struct astnode_charlit){NT_CHARLIT,NULL,NULL,$1};}
		;

/* primary expr: 6.5.1 */
pexpr:		IDENT		{ALLOC_SET_IDENT($$,$1);}
		| constant	{$$=$1;}
		| STRING	{ALLOC($$);$$->string=(struct astnode_string){NT_STRING,NULL,NULL,$1};}
		| '(' expr ')'	{$$=$2;}
		;

/* postfix expression: 6.5.2*/
pofexpr:	pexpr				{$$=$1;}
		| pofexpr '[' expr ']'		{/* rewrite a[b]=*(a+b) */
						 union astnode *inner;
						 ALLOC_SET_BINOP(inner,'+',$1,$3);
						 ALLOC_SET_UNOP($$,'*',inner);}
		| pofexpr '(' arglistopt ')'	{ALLOC($$);
						 $$->fncall=(struct astnode_fncall){NT_FNCALL,NULL,NULL,$1,$3};}
		| pofexpr '.' IDENT		{union astnode *ident;
						 ALLOC_SET_IDENT(ident,$3);
						 ALLOC_SET_BINOP($$,$2,$1,ident);}
		| pofexpr INDSEL IDENT		{union astnode *ident;
						 ALLOC_SET_IDENT(ident,$3);
						 ALLOC_SET_BINOP($$,$2,$1,ident);}
		| pofexpr PLUSPLUS		{/*a++ <=> */ALLOC_SET_UNOP($$,$2,$1);}
		| pofexpr MINUSMINUS		{ALLOC_SET_UNOP($$,$2,$1);}
		/*| '(' typename ')' '{' initlist '}'	{TODO}
		| '(' typename ')' '{' initlist ',' '}'	{TODO}*/
		;

arglist:	asnmtexpr			{$$=$1;}
		| arglist ',' asnmtexpr		{$$=$1;$1->generic.next=$3;}
		;

arglistopt:	arglist				{$$=$1;}
		|				{$$=NULL;}
		;

/* unary operators: 6.5.3; doesn't include C11 _Alignof */
uexpr:		pofexpr			{$$=$1;}
		| PLUSPLUS uexpr	{/*replace ++a with a=a+1*/
					 union astnode *one, *inner;
					 ALLOC(one);
					 one->num=(struct astnode_number){NT_NUMBER,NULL,NULL,(struct number){INT_T,UNSIGNED_T,1}};
					 ALLOC_SET_BINOP(inner,'+',$2,one);
					 ALLOC_SET_BINOP($$,'=',$2,inner);}
		| MINUSMINUS uexpr	{/*replace --a with a=a-1*/
					 union astnode *one, *inner;
					 ALLOC(one);
					 one->num=(struct astnode_number){NT_NUMBER,NULL,NULL,(struct number){INT_T,UNSIGNED_T,1}};
					 ALLOC_SET_BINOP(inner,'-',$2,one);
					 ALLOC_SET_BINOP($$,'=',$2,inner);}
		| uop castexpr		{ALLOC_SET_UNOP($$,$1,$2);}
		/*| SIZEOF uexpr		{TODO}
		| SIZEOF '(' typename ')'	{TODO}*/
		;

uop:		'&'		{$$=$1;}
		| '*'		{$$=$1;}	
		| '+'		{$$=$1;}
		| '-'		{$$=$1;}
		| '~'		{$$=$1;}	
		| '!'		{$$=$1;}
		;

castexpr:	uexpr					{$$=$1;}
		/*| '(' typename ')' castexpr {TODO}*/
		;

multexpr:	castexpr		{$$=$1;}
		| multexpr '*' multexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| multexpr '/' multexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| multexpr '%' multexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

addexpr:	multexpr		{$$=$1;}
		| addexpr '+' multexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| addexpr '-' multexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;
		

shftexpr:	addexpr			{$$=$1;}
		| shftexpr SHL shftexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| shftexpr SHR shftexpr {ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

relexpr:	shftexpr		{$$=$1;}
		| relexpr '<' shftexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| relexpr '>' shftexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| relexpr LTEQ shftexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| relexpr GTEQ shftexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

eqexpr:		relexpr			{$$=$1;}
		| eqexpr EQEQ eqexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| eqexpr NOTEQ eqexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

andexpr:	eqexpr			{$$=$1;}
		| andexpr '&' eqexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

xorexpr:	andexpr			{$$=$1;}
		| xorexpr '^' andexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

orexpr:		xorexpr			{$$=$1;}
		| orexpr '|' xorexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

logandexpr:	orexpr			{$$=$1;}
		| logandexpr LOGAND orexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

logorexpr:	logandexpr		{$$=$1;}
		| logorexpr LOGOR logandexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

condexpr:	logorexpr		{$$=$1;}
		| logorexpr '?' expr ':' condexpr {ALLOC_SET_TERNOP($$, $1, $3, $5);}
		;

asnmtexpr:	condexpr			{$$=$1;}
		| uexpr '=' asnmtexpr		{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| uexpr TIMESEQ asnmtexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| uexpr DIVEQ asnmtexpr		{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| uexpr MODEQ asnmtexpr		{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| uexpr PLUSEQ asnmtexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| uexpr MINUSEQ asnmtexpr	{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| uexpr SHLEQ asnmtexpr		{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| uexpr SHREQ asnmtexpr		{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| uexpr ANDEQ asnmtexpr		{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| uexpr XOREQ asnmtexpr		{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| uexpr OREQ asnmtexpr		{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

/*comma expression*/
expr:	asnmtexpr			{$$=$1;}
		| expr ',' asnmtexpr	{$$=$1;$1->generic.next=$3;}
		;


%%
/*
expr:	IDENT			{ALLOC($$);$$->ident=(struct astnode_ident)
				 {NT_IDENT,$1};}
	| NUMBER		{ALLOC($$);
				 $$->num=(struct astnode_number)
				 {0,(struct number){}};}
	| expr '+' expr		{ALLOC_SET_BINOP($$, '+', $1, $3);}
	| expr '-' expr		{ALLOC_SET_BINOP($$, '-', $1, $3);}
	| expr '*' expr		{ALLOC_SET_BINOP($$, '*', $1, $3);}
	| expr '/' expr		{/* TODO: check for division by zero if
				 $3 is a literal zero constant *\/
				 if (!$3) fprintf(stderr, "/0 err\n");
				 else ALLOC_SET_BINOP($$, '/', $1, $3);}
	| '+' expr %prec '!'    {ALLOC_SET_UNOP($$, '+', $2);}
	| '-' expr %prec '!'    {ALLOC_SET_UNOP($$, '-', $2);}
	| '*' expr %prec '!'    {ALLOC_SET_UNOP($$, '*', $2);}
	| '&' expr %prec '!'    {ALLOC_SET_UNOP($$, '&', $2);}
	| '(' expr ')'		{$$=$2;}
	;
*/

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
			fprintf(stdout, "op: %c\n", node->binop.op);
			print_astnode_recursive(node->binop.left, depth+1);
			print_astnode_recursive(node->binop.right, depth+1);
			break;
		case NT_UNOP:
			INDENT(depth);
			fprintf(stdout, "op: %c\n", node->unop.op);
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