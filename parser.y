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

%right	'?' ':'		/* ternary */
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

%type <astnode>	pexpr
%type <ident> IDENT
%%
exprlist:	pexpr		{print_astnode($1);}

/* 6.4.4.3 */
enumconst:	IDENT		{/* TODO: identifier declared as an
				 enum constant; might cause errors later by
				 conflicting with IDENT? */}
		;

/* 6.4.4 */
constant:	NUMBER		{/* TODO */}
		| enumconst	{/* TODO */}
		| CHARLIT	{/* TODO */}
		;

/* primary expr: 6.5.1; doesn't include C11 generic selections */
pexpr:		IDENT		{/* TODO */}
		| constant	{/* TODO */}
		| STRING	{/* TODO */}
		| '(' pexpr ')'	{/* TODO */}
		;

/* postfix expression: 6.5.2*/
pofexpr:	pexpr				{/*TODO*/}
		| pofexpr '[' expr ']'		{/*TODO*/}
		| pofexpr '(' arglistopt ')'	{/*TODO*/}
		| pofexpr '.' IDENT		{/*TODO*/}
		| pofexpr INDSEL IDENT		{/*TODO*/}
		| pofexpr PLUSPLUS		{/*TODO*/}
		| pofexpr MINUSMINUS		{/*TODO*/}
		| '(' typename ')' '{' initlist '}'	{/*TODO*/}
		| '(' typename ')' '{' initlist ',' '}'	{/*TODO*/}
		;

arglist:	asnmtexpr		{/*TODO*/}
		| arglist ',' asnmtexpr	{/*TODO*/}
		|			{/*TODO*/}
		;

arglistopt:	arglist		{/* TODO */}
		|		{/* empty */}
		;

/* unary operators: 6.5.3; doesn't include C11 _Alignof */
uexpr:		pofexpr
		| PLUSPLUS uexpr	{/*TODO*/}
		| MINUSMINUS uexpr	{/*TODO*/}
		| uop castexpr		{/*TODO*/}
		| SIZEOF uexpr		{/*TODO*/}
		| SIZEOF '(' typename ')'	{/*TODO*/}
		;

uop:		'&'		{/*TODO*/}
		| '*'		{/*TODO*/}	
		| '+'		{/*TODO*/}
		| '-'		{/*TODO*/}
		| '~'		{/*TODO*/}	
		| '!'		{/*TODO*/}
		;

castexpr:	uexpr
		| '(' typename ')' castexpr {/*TODO*/}
		;

multexpr:	castexpr		{/*TODO*/}
		| multexpr '*' multexpr	{/*TODO*/}
		| multexpr '/' multexpr	{/*TODO*/}
		| multexpr '%' multexpr	{/*TODO*/}
		;

addexpr:	castexpr
		| addexpr '+' multexpr {/*TODO*/}
		| addexpr '-' multexpr {/*TODO*/}
		;
		

shftexpr:	addexpr			{/*TODO*/}
		| shftexpr SHL shftexpr	{/*TODO*/}
		| shftexpr SHR shftexpr {/*TODO*/}
		;

relexpr:	shftexpr	{/*TODO*/}
		| relexpr '<' shftexpr {/*TODO*/}
		| relexpr '>' shftexpr {/*TODO*/}
		| relexpr LTEQ shftexpr {/*TODO*/}
		| relexpr GTEQ shftexpr {/*TODO*/}
		;


eqexpr:		relexpr			{/*TODO*/}
		| eqexpr EQEQ eqexpr	{/*TODO*/}
		| eqexpr NOTEQ eqexpr	{/*TODO*/}
		;

andexpr:	eqexpr			{/*TODO*/}
		| andexpr '&' andexpr	{/*TODO*/}
		;

xorexpr:	andexpr			{/*TODO*/}
		| orexpr '^' andexpr	{/*TODO*/}
		;

orexpr:		orexpr			{/*TODO*/}
		| orexpr '|' xorexpr	{/*TODO*/}
		;

logandexpr:	orexpr			{/*TODO*/}
		| logandexpr LOGAND orexpr	{/*TODO*/}
		;

logorexpr:	logandexpr		{/*TODO*/}
		| logorexpr LOGOR logandexpr	{/*TODO*/}
		;

condexpr:	logorexpr
		| logorexpr '?' expr ':' condexpr {/*TODO*/}
		;

asnmtexpr:	condexpr
		| uexpr '=' asnmtexpr	{/*TODO*/}
		| uexpr TIMESEQ asnmtexpr	{/*TODO*/}
		| uexpr DIVEQ asnmtexpr	{/*TODO*/}
		| uexpr MODEQ asnmtexpr	{/*TODO*/}
		| uexpr PLUSEQ asnmtexpr	{/*TODO*/}
		| uexpr MINUSEQ asnmtexpr	{/*TODO*/}
		| uexpr SHLEQ asnmtexpr	{/*TODO*/}
		| uexpr SHREQ asnmtexpr	{/*TODO*/}
		| uexpr ANDEQ asnmtexpr	{/*TODO*/}
		| uexpr XOREQ asnmtexpr	{/*TODO*/}
		| uexpr OREQ asnmtexpr	{/*TODO*/}
		;

/*comma expression*/
expr:	asnmtexpr
		| expr ',' asnmtexpr	{/*TODO*/}
		;

/* 6.6 constant expressions */
/* TODO: semantic parsing? */
constexpr:	condexpr	{/*TODO*/}
		;

/* 6.7 DECLARATIONS */
decl:	declspec			{/*TODO*/}
	| declspec initdecllist	';'	{/*TODO*/}
	;

/* declaration specifier */
declspec:	scspec 			{/*TODO*/}
		| typespec 		{/*TODO*/}
		| typequal 		{/*TODO*/}
		| funcspec 		{/*TODO*/}
		| declspec declspec	{/*TODO*/}
		;

initdecllist:	initdecl		{/*TODO*/}
		| initdecllist ',' initdecl	{/*TODO*/}
		;

initdecl:	declarator
		| declarator '=' initializer	{/*TODO*/}
		;

/* 6.7.1 storage class specifier */
scspec:		TYPEDEF		{/*TODO*/}
		| EXTERN		{/*TODO*/}
		| STATIC		{/*TODO*/}
		| AUTO		{/*TODO*/}
		| REGISTER		{/*TODO*/}
		;

/* 6.7.2 type specifiers */
typespec:	VOID		{/*TODO*/}
		| CHAR		{/*TODO*/}	
		| SHORT		{/*TODO*/}
		| INT		{/*TODO*/}	
		| LONG		{/*TODO*/}	
		| FLOAT		{/*TODO*/}
		| DOUBLE	{/*TODO*/}	
		| SIGNED	{/*TODO*/}	
		| UNSIGNED	{/*TODO*/}	
		| _BOOL		{/*TODO*/}
		| _COMPLEX	{/*TODO*/}	
		| structunionspec	{/*TODO*/}
		| enumspec	{/*TODO*/}	
		| typedefname	{/*TODO*/}
		;

/* 6.7.2.1 structure and union specifiers */
structunionspec:structunion '{' structdecllist '}'		{/*TODO*/}
		| structunion IDENT '{' structdecllist '}'	{/*TODO*/}
		| structunion IDENT				{/*TODO*/}
		;

structunion:	STRUCT		{/*TODO*/}
		| UNION		{/*TODO*/}
		;

structdecllist:	structdecl	{/*TODO*/}
		| structdecllist structdecl	{/*TODO*/}
		;

structdecl:	specquallist structdecllist ';'	{/*TODO*/}
		;

specquallist:	typespec specquallist		{/*TODO*/}
		| typespec			{/*TODO*/}
		| typequal specquallist		{/*TODO*/}
		| typequal			{/*TODO*/}
		;

structdecllist:	structdecl
		| structdecllist ',' structdecllist	{/*TODO*/}
		;

structdecl:	declarator			{/*TODO*/}
		| declarator ':' constexpr	{/*TODO*/}
		| ':' constexpr			{/*TODO*/}
		;

enumspec:	ENUM IDENT '{' enumlist '}'		{/*TODO*/}
		| ENUM IDENT '{' enumlist ',' '}'		{/*TODO*/}
		| ENUM '{' enumlist '}'		{/*TODO*/}
		| ENUM '{' enumlist ',' '}'		{/*TODO*/}
		| ENUM IDENT		{/*TODO*/}
		;

enumlist:	enumrtr
		| enumlist ',' enumrtr {/*TODO*/}
		;
enumrtr:	enumconst
		| enumconst '=' constexpr	{/*TODO*/}
		;

/* type qualifiers */
typequal:	CONST			{/*TODO*/}
		| RESTRICT		{/*TODO*/}
		| VOLATILE		{/*TODO*/}
		;

/* 6.7.4 Function Declaration */
funcspec:	INLINE			{/*TODO*/}
		;

/* 6.7.5 Declarators */
declarator:	pointer dirdeclarator	{/*TODO*/}
		| pointer		{/*TODO*/}
		;

dirdeclarator:	IDENT							{/*TODO*/}
		| '(' declarator ')'					{/*TODO*/}
		| dirdeclarator '[' typequallistopt asnmtexpr ']'	{/*TODO*/}
		| dirdeclarator '[' typequallistopt ']'			{/*TODO*/}
		| dirdeclarator '[' STATIC typequallistopt asnmtexpr ']'	{/*TODO*/}
		| dirdeclarator '[' STATIC typequallistopt ']'		{/*TODO*/}
		| dirdeclarator '[' typequallist STATIC asnmtexpr ']'	{/*TODO*/}
		| dirdeclarator '[' typequallistopt '*' ']'		{/*TODO*/}
		| dirdeclarator '(' paramtypelist ')'			{/*TODO*/}
		| dirdeclarator '(' identlist ')'			{/*TODO*/}
		| dirdeclarator '(' ')'					{/*TODO*/}
		;

pointer:	'*' typequallistopt	{/*TODO*/}
		| '*' typequallistopt pointer	{/*TODO*/}
		;

typequallistopt:typequallist		{/*TODO*/}
		|			{/*empty*/}
		;

paramtypelist:	paramlist
		| paramlist ',' ELLIPSIS	{/*TODO*/}
		;

paramlist:	paramdecl			{/*TODO*/}
		| paramlist ',' paramdecl	{/*TODO*/}
		;

paramdecl:	declspec declarator		{/*TODO*/}
		| declspec abstdeclr		{/*TODO*/}
		| declspec 					{/*TODO*/}
		;

identlist:	IDENT
		| identlist ',' IDENT
		;

/*
// these are all valid
const const int const const i;
int const i;
const int *const i, *const *const (j);
const int const i, const j;
*/

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