%debug
%{
#define YYDEBUG	1
#define parse.error verbose

#include "parser.h"
#include "lexerutils/errorutils.h"
#include "astnode.h"
#include "asttypes.h"
%}
%union {
	// from lexer
	int sc;	// single-character tokens
	struct number num;
	char *ident;
	struct string string;
	struct charlit charlit;
	
	// from syntax parsing
	union astnode *astnode;		// abstract syntax tree

}

%token		IDENT NUMBER CHARLIT STRING
%token<sc>	INDSEL PLUSPLUS MINUSMINUS SHL
%token<sc>	SHR LTEQ GTEQ EQEQ NOTEQ LOGAND LOGOR ELLIPSIS TIMESEQ DIVEQ
%token<sc>	MODEQ PLUSEQ MINUSEQ SHLEQ SHREQ ANDEQ OREQ XOREQ AUTO BREAK
%token<sc>	CASE CHAR CONST CONTINUE DEFAULT DO DOUBLE ELSE ENUM EXTERN
%token<sc>	FLOAT FOR GOTO IF INLINE INT LONG REGISTER RESTRICT RETURN
%token<sc>	SHORT SIGNED SIZEOF STATIC STRUCT SWITCH TYPEDEF UNION
%token<sc>	UNSIGNED VOID VOLATILE WHILE _BOOL _COMPLEX _IMAGINARY

%token<sc>	',' '=' '?' ':' '|' '^' '&' '<' '>' '+' '-' '*' '/' '%' '!' '~'
%token<sc>	'(' ')' '[' ']' '.'

/* reference: https://en.cppreference.com/w/c/language/operator_precedence
 * these are redundant because of rule hierarchy but still nice to have in
 * one place */
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
%left	'!' '~' SIZEOF MINUSMINUS PLUSPLUS

/* this precedence level also includes pre(inc|dec)rement */
%left	'(' ')' '[' ']' '.' INDSEL

/* to correctly parse nested if...else statements */
%left IF
%left ELSE

%type<sc> 	uop
%type<astnode>	constant pexpr pofexpr arglist arglistopt uexpr
%type<astnode>	castexpr multexpr addexpr shftexpr relexpr eqexpr andexpr
%type<astnode>	xorexpr orexpr logandexpr logorexpr condexpr asnmtexpr expr
%type<astnode>	decl initdecllist initdecl typespec scspec declspec typequal
%type<astnode>	pointer dirdeclarator typequallist typequallistopt declarator
%type<astnode>	asnmtexpropt paramlist paramtypelist paramdecl absdeclarator
%type<astnode>	specquallist typename dirabsdeclarator dirabsdeclaratoropt
%type<astnode>	paramtypelistopt
%type<ident> 	IDENT
%type<string>	STRING
%type<charlit>	CHARLIT
%type<num>	NUMBER
%%
exprstmt:	expr ';'		{print_astnode($1);}
		| exprstmt expr ';'	{print_astnode($2);}
		| decl			{/*TODO*/}
		;

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
		| pofexpr '[' expr ']'		{/* rewrite a[b] <=> *(a+b) */
						 union astnode *inner;
						 ALLOC_SET_BINOP(inner,'+',$1,$3);
						 ALLOC_SET_UNOP($$,'*',inner);}
		| pofexpr '(' arglistopt ')'	{ALLOC($$);
						 $$->fncall=(struct astnode_fncall){NT_FNCALL,NULL,NULL,$1,$3};}
		| pofexpr '.' IDENT		{union astnode *ident;
						 ALLOC_SET_IDENT(ident,$3);
						 ALLOC_SET_BINOP($$,$2,$1,ident);}
		| pofexpr INDSEL IDENT		{/* rewrite a->b <=> (*a).b */
						 union astnode *ident, *deref;
						 ALLOC_SET_IDENT(ident,$3);
						 ALLOC_SET_UNOP(deref,'*',$1);
						 ALLOC_SET_BINOP($$,'.',deref,ident);}
		| pofexpr PLUSPLUS		{/*a++ <=> */ALLOC_SET_UNOP($$,$2,$1);}
		| pofexpr MINUSMINUS		{ALLOC_SET_UNOP($$,$2,$1);}
		/*| '(' typename ')' '{' initlist '}'	{TODO}
		| '(' typename ')' '{' initlist ',' '}'	{TODO}*/
		;

arglist:	asnmtexpr			{$$=$1;}
		| arglist ',' asnmtexpr		{$$=$1;
						 /* append $3 to end of linked list*/
						 union astnode *argnode=$1;
						 while (argnode->generic.next) argnode = argnode->generic.next;
						 argnode->generic.next=$3;}
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
		| SIZEOF uexpr		{ALLOC_SET_UNOP($$,$1,$2);}
		/*| SIZEOF '(' typename ')'	{TODO}*/
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
		| uexpr TIMESEQ asnmtexpr	{ASNEQ($$,'*',$1,$3);}
		| uexpr DIVEQ asnmtexpr		{ASNEQ($$,'/',$1,$3);}
		| uexpr MODEQ asnmtexpr		{ASNEQ($$,'%',$1,$3);}
		| uexpr PLUSEQ asnmtexpr	{ASNEQ($$,'+',$1,$3);}
		| uexpr MINUSEQ asnmtexpr	{ASNEQ($$,'-',$1,$3);}
		| uexpr SHLEQ asnmtexpr		{ASNEQ($$,SHL,$1,$3);}
		| uexpr SHREQ asnmtexpr		{ASNEQ($$,SHR,$1,$3);}
		| uexpr ANDEQ asnmtexpr		{ASNEQ($$,'&',$1,$3);}
		| uexpr XOREQ asnmtexpr		{ASNEQ($$,'^',$1,$3);}
		| uexpr OREQ asnmtexpr		{ASNEQ($$,'|',$1,$3);}
		;

/*comma expression*/
expr:	asnmtexpr				{$$=$1;}
		| expr ',' asnmtexpr		{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

/* 6.7 DECLARATIONS */
decl:	declspec ';'				{/*nothing to do here*/}
	| declspec initdecllist	';'		{/*nothing to do here*/}
	;

/* declaration specifier */
declspec:	scspec 				{ALLOC_DECLSPEC($$);$$->declspec.sc=$1;}
		| typespec 			{ALLOC_DECLSPEC($$);$$->declspec.ts=$1;}
		| typequal 			{ALLOC_DECLSPEC($$);$$->declspec.tq=$1;}
		| funcspec 			{/*the only funcspec is INLINE; ignore*/}
		| declspec declspec		{$$=merge_declspec($1,$2);}
		;

initdecllist:	initdecl			{$$=$1;}
		| initdecllist ',' initdecl	{$$=$1;LL_APPEND($1,$3);}
		;

initdecl:	declarator			{/*insert into symtab*/}
		| declarator '=' initializer	{/*add initializer to variable,insert declarator into symtab*/}
		;

/* 6.7.1 storage class specifier */
scspec:		TYPEDEF				{/*TODO*/}
		| EXTERN			{ALLOC_SET_SCSPEC($$,SC_EXTERN);}
		| STATIC			{ALLOC_SET_SCSPEC($$,SC_STATIC);}
		| AUTO				{ALLOC_SET_SCSPEC($$,SC_AUTO);}
		| REGISTER			{ALLOC_SET_SCSPEC($$,SC_REGISTER);}
		;

/* 6.7.2 type specifiers */
typespec:	VOID				{ALLOC_SET_SCALAR($$,BT_VOID,LLS_UNSPEC,SIGN_UNSPEC);}
		| CHAR				{ALLOC_SET_SCALAR($$,BT_CHAR,LLS_UNSPEC,SIGN_UNSPEC);}	
		| SHORT				{ALLOC_SET_SCALAR($$,BT_UNSPEC,LLS_SHORT,SIGN_UNSPEC);}	
		| INT				{ALLOC_SET_SCALAR($$,BT_INT,LLS_UNSPEC,SIGN_UNSPEC);}
		| LONG				{ALLOC_SET_SCALAR($$,BT_UNSPEC,LLS_LONG,SIGN_UNSPEC);}
		| FLOAT				{ALLOC_SET_SCALAR($$,BT_FLOAT,LLS_UNSPEC,SIGN_UNSPEC);}
		| DOUBLE			{ALLOC_SET_SCALAR($$,BT_DOUBLE,LLS_UNSPEC,SIGN_UNSPEC);}
		| SIGNED 			{ALLOC_SET_SCALAR($$, BT_UNSPEC, LLS_UNSPEC, SIGN_SIGNED);}
		| UNSIGNED			{ALLOC_SET_SCALAR($$, BT_UNSPEC, LLS_UNSPEC, SIGN_UNSIGNED);}
		| _BOOL 			{ALLOC_SET_SCALAR($$,BT_BOOL,LLS_UNSPEC,SIGN_UNSPEC);}
		| _COMPLEX 			{/*TODO: might not implement this*/}
		| structunionspec		{/*TODO*/}
		| enumspec			{/*TODO*/}	
		| typedefname			{/*TODO*/}
		;


/* 6.7.2.1 structure and union specifiers */
structunionspec:structunion '{' structdecllist '}'		{/*TODO*/}
		| structunion IDENT '{' structdecllist '}'	{/*TODO*/}
		| structunion IDENT				{/*TODO*/}
		;

structunion:	STRUCT				{/*TODO*/}
		| UNION				{/*TODO*/}
		;

structdecllist:	structdecl			{/*TODO*/}
		| structdecllist structdecl	{/*TODO*/}
		;

structdecl:	specquallist structdecllist ';'	{/*TODO*/}
		;

/*dummy rules: TODO*/
enumspec:;
funcspec:;
initializer:;
structunionspec:;

specquallist:	typespec			{ALLOC_DECLSPEC($$);$$->declspec->ts=$1;}
		| typequal			{ALLOC_DECLSPEC($$);$$->declspec->tq=$1;}
		| specquallist specquallist	{$$=merge_declspec($1,$2);}
		;

/* 6.7.3 type qualifiers */
typequal:	CONST				{ALLOC_SET_TQSPEC($$,TQ_CONST);}
		| RESTRICT			{ALLOC_SET_TQSPEC($$,TQ_RESTRICT);}
		| VOLATILE			{ALLOC_SET_TQSPEC($$,TQ_VOLATILE);}
		;

/* 6.7.4 Function Declaration */
funcspec:	INLINE				{/*NOT DEALING WITH THIS*/}
		;


/* 6.7.5 Declarators */
declarator:	pointer dirdeclarator		{ALLOC_DECLARATOR($$,$2,$1,0);$$=$1;}
		| dirdeclarator			{ALLOC_DECLARATOR($$,$1,NULL,0);}
		;

dirdeclarator:	IDENT							{union astnode *ident;
									 ALLOC_SET_IDENT(ident,$1);
									 ALLOC_REGULAR_DIRDECLARATOR($$,ident);}
		| '(' declarator ')'					{$$=$2;}
		| dirdeclarator '[' typequallistopt asnmtexpropt ']'	{ALLOC_ARRAY_DIRDECLARATOR($$,$1,$3,$4,0);}
		| dirdeclarator '[' typequallistopt ']'			{ALLOC_ARRAY_DIRDECLARATOR($$,$1,$3,NULL,0);}
		| dirdeclarator '[' STATIC typequallistopt asnmtexpr ']'{/*ignore static keyword*/
									 ALLOC_ARRAY_DIRDECLARATOR($$,$1,$4,$5,0);}
		| dirdeclarator '[' STATIC typequallistopt ']'		{ALLOC_ARRAY_DIRDECLARATOR($$,$1,$4,NULL,0);}
		| dirdeclarator '[' typequallist STATIC asnmtexpr ']'	{ALLOC_ARRAY_DIRDECLARATOR($$,$1,$3,$5,0);}
		| dirdeclarator '[' typequallistopt '*' ']'		{/*ignore variable length array*/
									 ALLOC_ARRAY_DIRDECLARATOR($$,$1,$3,NULL,0);}
		| dirdeclarator '(' paramtypelist ')'			{ALLOC_FN_DIRDECLARATOR($$,$1,$3,0);}
		| dirdeclarator '(' identlist ')'			{/*ignore old C function syntax*/
									 /*TODO: throw an error because this may cause other problems*/}
		| dirdeclarator '(' ')'					{ALLOC_FN_DIRDECLARATOR($$,$1,NULL,0);}
		;

pointer:	'*' typequallistopt		{ALLOC_POINTER($$,$2,NULL);}
		| '*' typequallistopt pointer	{ALLOC_POINTER($$,$2,$3);$$=$3;}
		;

typequallist:	typequal			{$$=$1;}
		| typequallist typequal		{$$=$1;LL_APPEND($1,$2);}
		;

typequallistopt:typequallist			{$$=$1;}
		|				{$$=NULL;}
		;

paramtypelist:	paramlist			{$$=$1;}
		| paramlist ',' ELLIPSIS	{$$=$1;LL_APPEND($1,&ELLIPSIS_DECLARATOR);}
		;

paramlist:	paramdecl			{$$=$1;}
		| paramlist ',' paramdecl	{$$=$1;LL_APPEND($1,$3);}
		;

paramdecl:	declspec declarator		{ALLOC_PARAMDECL($$,$1,$2);}
		| declspec absdeclarator	{ALLOC_PARAMDECL($$,$1,$2);}
		| declspec 			{ALLOC_PARAMDECL($$,$1,NULL);}
		;

identlist:	IDENT				{/*NOT DEALING WITH THIS*/}
		| identlist ',' IDENT		{/*ONLY FOR OLD C FN SYNTAX*/}		
		;

/* 6.7.6: type names */
typename:	specquallist absdeclarator	{ALLOC_TYPENAME($$,$1,$2);}
		| specquallist			{ALLOC_TYPENAME($$,$1,NULL);}
		;

absdeclarator:	pointer				{ALLOC_DECLARATOR($$,NULL,$1,1);}
		| pointer dirabsdeclarator	{ALLOC_DECLARATOR($$,$2,$1,1);}
		| dirabsdeclarator		{ALLOC_DECLARATOR($$,$1,NULL,1);}
		;

dirabsdeclarator: '(' absdeclarator ')'						{$$=$2;}
		| dirabsdeclaratoropt '[' typequallistopt asnmtexpropt ']'	{ALLOC_ARRAY_DIRDECLARATOR($$,$1,$3,$4,1);}
		| dirabsdeclaratoropt '[' STATIC typequallistopt asnmtexpropt ']'{ALLOC_ARRAY_DIRDECLARATOR($$,$1,$4,$5,1);}
		| dirabsdeclaratoropt '[' typequallist STATIC asnmtexpr ']'	{ALLOC_ARRAY_DIRDECLARATOR($$,$1,$3,$5,1);}
		| dirabsdeclaratoropt '[' '*' ']'				{/*ignore/throw error on VLA*/}
		| dirabsdeclaratoropt '(' paramtypelistopt ')'			{ALLOC_FN_DIRDECLARATOR($$,$1,$3,1);}
		;

dirabsdeclaratoropt:	dirabsdeclarator	{$$=$1;}
		| 				{$$=NULL;}
		;

asnmtexpropt:	asnmtexpr			{$$=$1;}
		| 				{$$=NULL;}
		;

paramtypelistopt:	paramtypelist		{$$=$1;}
		|				{$$=NULL;}
		;

/* 6.7.7 type definitions */
typedefname:	IDENT				{/*TODO: kludges*/}
		;

%%

int main()
{
	yydebug = 0;
	yyparse();
}

int yyerror(char *err)
{
	fprintf(stderr, "Syntax err: %s\n", err);
}

// for indenting
int indi;
void print_astnode_rec(union astnode *node, int depth)
{
	INDENT(depth);
	switch (node->generic.type) {
	case NT_NUMBER:;
		char *numstring = print_number(node->num.num);
		fprintf(stdout, "CONSTANT:  %s\n", numstring);
		free(numstring);
		break;
	case NT_STRING:;
		char *strstring = print_string(&node->string.string);
		// assumes single-width, null-terminated strings
		fprintf(stdout, "STRING  %s\n", strstring);
		free(strstring);
		break;
	case NT_CHARLIT:;
		char chrstring[5];
		print_char(node->charlit.charlit.value.none, chrstring);
		// assumes single-width character literal
		fprintf(stdout, "CHARLIT  %s\n", chrstring);
		break;
	case NT_IDENT:
		fprintf(stdout, "IDENT  %s\n", node->ident.ident);
		break;
	case NT_BINOP:;
		int printsym = 1;
		switch (node->binop.op) {
		// these are differentiated in Hak's output
		case '=':
			fprintf(stdout, "ASSIGNMENT\n");
			printsym = 0;
			break;
		case '.':
			fprintf(stdout, "SELECT\n");
			printsym = 0;
			break;
		case EQEQ: case NOTEQ: case '>': case '<': case LTEQ: case GTEQ:
			fprintf(stdout, "COMPARISON  OP  ");
			break;
		// logical operators are differentiated in hak's output
		case LOGAND: case LOGOR:
			fprintf(stdout, "LOGICAL  OP  ");
			break;
		default:
			fprintf(stdout, "BINARY  OP  ");
		}
		if (node->binop.op <= 0xff && printsym) {
			fprintf(stdout, "%c\n", node->binop.op);
		} else if (printsym) {
			fprintf(stdout, "%s\n",
				toktostr(node->binop.op));
		}
		print_astnode_rec(node->binop.left, depth+1);
		print_astnode_rec(node->binop.right, depth+1);
		break;
	case NT_UNOP:
		switch (node->unop.op) {
		// these are differentiated in Hak's output
		case '*':
			fprintf(stdout, "DEREF\n");
			break;
		case '&':
			fprintf(stdout, "ADDRESSOF\n");
			break;
		case SIZEOF:
			fprintf(stdout, "SIZEOF\n");
			break;
		// ++ and -- are special cases: prefix forms
		// get rewritten, so these are specifically
		// postfix forms
		case PLUSPLUS: case MINUSMINUS:
			fprintf(stdout, "UNARY  OP  POST%s\n",
				node->unop.op==PLUSPLUS ? "INC" : "DEC");
			break;
		default:
			fprintf(stdout, "UNARY  OP  ");
			if (node->unop.op <= 0xff) {
				fprintf(stdout, "%c\n", node->unop.op);
			} else {
				fprintf(stdout, "%s\n",
					toktostr(node->unop.op));
			}
		}
		print_astnode_rec(node->unop.arg, depth+1);
		break;
	case NT_TERNOP:
		fprintf(stdout, "TERNARY  OP,  IF:\n");
		print_astnode_rec(node->ternop.first, depth+1);
		INDENT(depth);
		fprintf(stdout, "THEN:\n");
		print_astnode_rec(node->ternop.second, depth+1);
		INDENT(depth);
		fprintf(stdout, "ELSE:\n");
		print_astnode_rec(node->ternop.third, depth+1);
		break;
	case NT_FNCALL:;
		// count number of arguments
		int argc = 0;
		union astnode *argnode = node->fncall.arglist;
		while (argnode) {
			++argc;
			argnode = argnode->generic.next;
		}
		fprintf(stdout, "FNCALL,  %d  arguments\n", argc);

		// print function declarator
		print_astnode_rec(node->fncall.fnname, depth+1);

		// print arglist
		for (argc=0, argnode=node->fncall.arglist; argnode;
			++argc, argnode = argnode->generic.next) {
			INDENT(depth);
			fprintf(stdout, "arg  #%d=\n", argc+1);
			print_astnode_rec(argnode, depth+1);
		}
		break;
	default:
		fprintf(stdout, "AST type %d not implemented yet.\n",
			node->generic.type);
	}
}

void print_astnode(union astnode *node)
{
	print_astnode_rec(node, 0);
	fprintf(stdout, "\n");
}