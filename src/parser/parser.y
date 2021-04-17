%debug
%define parse.error verbose
%{
#define YYDEBUG 0

#include <parser/parser.h>
#include <common.h>
#include <lexer/errorutils.h>
#include <lexer/numutils.h>
#include <parser/astnode.h>
#include <parser/symtab.h>
#include <parser/scope.h>
#include <parser/structunion.h>
#include <parser/decl.h>
#include <parser/stmt.h>
#include <quads/quads.h>
#include <stdio.h>

int yydebug;
%}

%union {
	// from lexer
	int sc;	// single-character tokens

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
%type<astnode>	pointer dirdeclarator typequallist declarator
%type<astnode>	paramlist paramtypelist paramdecl absdeclarator declspeclist
%type<astnode>	specquallist typename dirabsdeclarator paramtypelistopt
%type<astnode>	structunionspec structunion structdeclaratorlist
%type<astnode>	structdeclarator specqual exprstmt fordecl labeledstmt
%type<astnode>	stmt compoundstmt selectionstmt iterationstmt jumpstmt
%type<astnode>	blockitemlist blockitem translnunit externdecl funcdef
%type<ident> 	IDENT
%type<string>	STRING
%type<charlit>	CHARLIT
%type<astnode>	NUMBER
%%
/* beware, the document gets wide here (rip 80 characters) */

/* top level is translation unit (from 6.9 external definitions) */
translnunit: 	externdecl 							{/*nothing to do*/}
		| translnunit externdecl					{/*nothing to do*/}
		;

/* 6.4.4.3 */
/*enumconst:	IDENT								{/*not implementing enums} */
		

/* 6.4.4 */
constant:	NUMBER								{$$=$1;}
		/*| enumconst							{/*not implementing enums}*/
		| CHARLIT							{ALLOC($$);$$->charlit=(struct astnode_charlit){NT_CHARLIT,NULL,$1};}
		;

/* primary expr: 6.5.1 */
pexpr:		IDENT								{$$=scope_lookup($1,NS_IDENT);
										 /*if not found indicate it with dummy pexpr*/
										 if(!$$){ALLOC_IMPL_FN($$,$1);}}
		| constant							{$$=$1;}
		| STRING							{ALLOC($$);$$->string=(struct astnode_string){NT_STRING,NULL,$1};}
		| '(' expr ')'							{$$=$2;}
		;

/* postfix expression: 6.5.2*/
pofexpr:	pexpr								{$$=$1;}
		| pofexpr '[' expr ']'						{/* rewrite a[b] <=> *(a+b) */
										 union astnode *inner;
										 CHECK_SYM_FOUND($1);
										 ALLOC_SET_BINOP(inner,'+',$1,$3);
										 ALLOC_SET_UNOP($$,'*',inner);}
		| pofexpr '(' arglistopt ')'					{ALLOC($$);
										 /*implicit declaration warning*/
										 CHECK_SYM_FOUND_WARN($1);
										 $$->fncall=(struct astnode_fncall){NT_FNCALL,NULL,$1,$3};}
		| pofexpr '.' IDENT						{union astnode *ident;
										 CHECK_SYM_FOUND($1);
										 ALLOC_SET_IDENT(ident,$3);
										 ALLOC_SET_BINOP($$,$2,$1,ident);}
		| pofexpr INDSEL IDENT						{/* rewrite a->b <=> (*a).b */
										 union astnode *ident, *deref;
										 CHECK_SYM_FOUND($1);
										 ALLOC_SET_IDENT(ident,$3);
										 ALLOC_SET_UNOP(deref,'*',$1);
										 ALLOC_SET_BINOP($$,'.',deref,ident);}
		| pofexpr PLUSPLUS						{CHECK_SYM_FOUND($1);
										 ALLOC_SET_UNOP($$,$2,$1);}
		| pofexpr MINUSMINUS						{CHECK_SYM_FOUND($1);
										 ALLOC_SET_UNOP($$,$2,$1);}
		/*| '(' typename ')' '{' initlist '}'				{not implementing compound literals}
		| '(' typename ')' '{' initlist ',' '}'				{not implementing compound literals}*/
		;

arglist:	asnmtexpr							{$$=$1;}
		| arglist ',' asnmtexpr						{$$=$1;LL_APPEND($1,$3);}
		;

arglistopt:	arglist								{$$=$1;}
		|								{$$=NULL;}
		;

/* unary operators: 6.5.3; doesn't include C11 _Alignof */
uexpr:		pofexpr								{$$=$1;
										 CHECK_SYM_FOUND($1);}
		| PLUSPLUS uexpr						{/*replace ++a with a=a+1*/
										 union astnode *inner;
										 /* TODO: remove this */
										 /*ALLOC(one);
										 one->num=(struct astnode_number){NT_NUMBER,NULL,
										 	(struct number){INT_T,SIGNED_T,1}};*/
										 ALLOC_SET_BINOP(inner,'+',$2,make_one());
										 ALLOC_SET_BINOP($$,'=',$2,inner);}
		| MINUSMINUS uexpr						{/*replace --a with a=a-1*/
										 union astnode *inner;
										 /* TODO: remove this
										 ALLOC(one);
										 one->num=(struct astnode_number){NT_NUMBER,NULL,
										 	(struct number){INT_T,SIGNED_T,1}};*/
										 ALLOC_SET_BINOP(inner,'-',$2,make_one());
										 ALLOC_SET_BINOP($$,'=',$2,inner);}
		| uop castexpr							{ALLOC_SET_UNOP($$,$1,$2);}
		| SIZEOF uexpr							{ALLOC_SET_UNOP($$,$1,$2);}
		| SIZEOF '(' typename ')'
										{/*use 's' to indicate sizeof with a type*/
										 ALLOC_SET_UNOP($$,'s',$3);}
		;

uop:		'&'								{$$=$1;}
		| '*'								{$$=$1;}
		| '+'								{$$=$1;}
		| '-'								{$$=$1;}
		| '~'								{$$=$1;}
		| '!'								{$$=$1;}
		;

castexpr:	uexpr								{$$=$1;}
		| '(' typename ')' castexpr					{/*use 'c' to indicate cast*/
										 ALLOC_SET_BINOP($$,'c',$2,$4);}
		;

multexpr:	castexpr							{$$=$1;}
		| multexpr '*' multexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| multexpr '/' multexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| multexpr '%' multexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

addexpr:	multexpr							{$$=$1;}
		| addexpr '+' multexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| addexpr '-' multexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;
		

shftexpr:	addexpr								{$$=$1;}
		| shftexpr SHL shftexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| shftexpr SHR shftexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

relexpr:	shftexpr							{$$=$1;}
		| relexpr '<' shftexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| relexpr '>' shftexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| relexpr LTEQ shftexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| relexpr GTEQ shftexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

eqexpr:		relexpr								{$$=$1;}
		| eqexpr EQEQ eqexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| eqexpr NOTEQ eqexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

andexpr:	eqexpr								{$$=$1;}
		| andexpr '&' eqexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

xorexpr:	andexpr								{$$=$1;}
		| xorexpr '^' andexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

orexpr:		xorexpr								{$$=$1;}
		| orexpr '|' xorexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

logandexpr:	orexpr								{$$=$1;}
		| logandexpr LOGAND orexpr					{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

logorexpr:	logandexpr							{$$=$1;}
		| logorexpr LOGOR logandexpr					{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

condexpr:	logorexpr							{$$=$1;}
		| logorexpr '?' expr ':' condexpr 				{ALLOC_SET_TERNOP($$, $1, $3, $5);}
		;

asnmtexpr:	condexpr							{$$=$1;}
		| uexpr '=' asnmtexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		| uexpr TIMESEQ asnmtexpr					{ASNEQ($$,'*',$1,$3);}
		| uexpr DIVEQ asnmtexpr						{ASNEQ($$,'/',$1,$3);}
		| uexpr MODEQ asnmtexpr						{ASNEQ($$,'%',$1,$3);}
		| uexpr PLUSEQ asnmtexpr					{ASNEQ($$,'+',$1,$3);}
		| uexpr MINUSEQ asnmtexpr					{ASNEQ($$,'-',$1,$3);}
		| uexpr SHLEQ asnmtexpr						{ASNEQ($$,SHL,$1,$3);}
		| uexpr SHREQ asnmtexpr						{ASNEQ($$,SHR,$1,$3);}
		| uexpr ANDEQ asnmtexpr						{ASNEQ($$,'&',$1,$3);}
		| uexpr XOREQ asnmtexpr						{ASNEQ($$,'^',$1,$3);}
		| uexpr OREQ asnmtexpr						{ASNEQ($$,'|',$1,$3);}
		;

/*comma expression*/
expr:		asnmtexpr							{$$=$1;}
		| expr ',' asnmtexpr						{ALLOC_SET_BINOP($$,$2,$1,$3);}
		;

/* 6.7 DECLARATIONS */
decl:		declspeclist ';'						{$$=NULL;/*potential tag forward declaration;
										 see structunion.c and decl.h*/
										 declspec_check_empty($1);}
		| declspeclist initdecllist ';'					{$$=NULL;/*nothing to do here*/}
		;

/* declaration specifier */
declspeclist:	declspec							{$$=$1;}
		| declspeclist declspec						{$$=merge_declspec($1,$2);}

declspec:	scspec 								{ALLOC_DECLSPEC($$);$$->declspec.sc=$1;}
		| typespec 							{ALLOC_DECLSPEC($$);$$->declspec.ts=$1;}
		| typequal 							{ALLOC_DECLSPEC($$);$$->declspec.tq=$1;}
		| funcspec 							{/*the only funcspec is INLINE; ignore*/}
		;

initdecllist:	initdecl							{/*doesn't have to return anything*/
										 decl_install($1,$<astnode>0);}
		| initdecllist ',' initdecl					{decl_install($3,$<astnode>0);}
		;

initdecl:	declarator							{$$=$1;}
		/*| declarator '=' initializer					{$$=$1;/*don't handle initializer}*/
		;

/* 6.7.1 storage class specifier */
scspec:		TYPEDEF								{/*not implementing this*/}
		| EXTERN							{ALLOC_SET_SCSPEC($$,SC_EXTERN);}
		| STATIC							{ALLOC_SET_SCSPEC($$,SC_STATIC);}
		| AUTO								{ALLOC_SET_SCSPEC($$,SC_AUTO);}
		| REGISTER							{ALLOC_SET_SCSPEC($$,SC_REGISTER);}
		;

/* 6.7.2 type specifiers */
typespec:	VOID								{ALLOC_SET_SCALAR($$,BT_VOID,LLS_UNSPEC,SIGN_UNSPEC);}
		| CHAR								{ALLOC_SET_SCALAR($$,BT_CHAR,LLS_UNSPEC,SIGN_UNSPEC);}
		| SHORT								{ALLOC_SET_SCALAR($$,BT_UNSPEC,LLS_SHORT,SIGN_UNSPEC);}
		| INT								{ALLOC_SET_SCALAR($$,BT_INT,LLS_UNSPEC,SIGN_UNSPEC);}
		| LONG								{ALLOC_SET_SCALAR($$,BT_UNSPEC,LLS_LONG,SIGN_UNSPEC);}
		| FLOAT								{ALLOC_SET_SCALAR($$,BT_FLOAT,LLS_UNSPEC,SIGN_UNSPEC);}
		| DOUBLE							{ALLOC_SET_SCALAR($$,BT_DOUBLE,LLS_UNSPEC,SIGN_UNSPEC);}
		| SIGNED 							{ALLOC_SET_SCALAR($$,BT_UNSPEC,LLS_UNSPEC,SIGN_SIGNED);}
		| UNSIGNED							{ALLOC_SET_SCALAR($$,BT_UNSPEC,LLS_UNSPEC,SIGN_UNSIGNED);}
		| _BOOL 							{ALLOC_SET_SCALAR($$,BT_BOOL,LLS_UNSPEC,SIGN_UNSPEC);}
		| _COMPLEX 							{/*not implementing this type*/}
		| structunionspec						{$$=$1;}
		/*| enumspec							{/*not implementing enums}*/
		/*| typedefname							{/*not implementing typedefs}*/
		;

/* 6.7.2.1 structure and union specifiers */
structunionspec:structunion {structunion_set_name(NULL,1);} '{' structdecllist '}'
										{$$=structunion_done(1);}
		| structunion IDENT {structunion_set_name($2,1);} '{' structdecllist '}'
										{$$=structunion_done(1);}
		| structunion IDENT						{structunion_set_name($2,0);
										 $$=structunion_done(0);}
		;

structunion:	STRUCT								{/*note: doesn't set $$, rather builds struct/union globally*/
										 structunion_new(SU_STRUCT);}
		| UNION								{structunion_new(SU_UNION);}
		;

structdecllist:	structdecl							{/*nothing to do here*/}
		| structdecllist structdecl					{/*nothing to do here*/}
		;

structdecl:	specquallist structdeclaratorlist ';'				{/*nothing to do here*/}
		;

specquallist:	specqual							{$$=$1;}
		| specquallist specqual						{$$=merge_declspec($1,$2);}
		;

specqual:	typespec							{ALLOC_DECLSPEC($$);$$->declspec.ts=$1;}
		| typequal							{ALLOC_DECLSPEC($$);$$->declspec.tq=$1;}
		;

structdeclaratorlist:	structdeclarator					{structunion_install_member($1,$<astnode>0);}
		| structdeclaratorlist ',' structdeclarator			{structunion_install_member($3,$<astnode>0);}
		;

structdeclarator:	declarator						{$$=$1;}
		/*| declarator ':' constexpr					{$$=$1;/*not implementing bitfields}*/
		/*| ':' constexpr						{/*not implementing bitfields}*/
		;

/* 6.7.3 type qualifiers */
typequal:	CONST								{ALLOC_SET_TQSPEC($$,TQ_CONST);}
		| RESTRICT							{ALLOC_SET_TQSPEC($$,TQ_RESTRICT);}
		| VOLATILE							{ALLOC_SET_TQSPEC($$,TQ_VOLATILE);}
		;

/* 6.7.4 Function Declaration */
funcspec:	INLINE								{/*not implementing inline functions*/}
		;


/* 6.7.5 Declarators */
declarator:	pointer dirdeclarator						{$$=decl_append($2,$1);}
		| dirdeclarator							{$$=$1;}
		;

/* have to expand these otherwise shift/reduce conflicts on optional sections */
dirdeclarator:	IDENT								{$$=decl_new($1);}
		| '(' declarator ')'						{$$=$2;}
		| dirdeclarator '[' typequallist asnmtexpr ']'			{$$=decl_append($1,decl_array_new($4,$3));}
		| dirdeclarator '[' asnmtexpr ']'				{$$=decl_append($1,decl_array_new($3,NULL));}
		| dirdeclarator '[' typequallist ']'				{$$=decl_append($1,decl_array_new(NULL,$3));}
		| dirdeclarator '[' ']'						{$$=decl_append($1,decl_array_new(NULL,NULL));}
		| dirdeclarator '[' STATIC typequallist asnmtexpr ']'		{/*ignore static keyword*/
										 $$=decl_append($1,decl_array_new($5,$4));}
		| dirdeclarator '[' STATIC asnmtexpr ']'			{$$=decl_append($1,decl_array_new($4,NULL));}
		| dirdeclarator '[' STATIC typequallist ']'			{$$=decl_append($1,decl_array_new(NULL,$4));}
		| dirdeclarator '[' STATIC ']'					{$$=decl_append($1,decl_array_new(NULL,NULL));}
		| dirdeclarator '[' typequallist STATIC asnmtexpr ']'		{$$=decl_append($1,decl_array_new($5,$3));}
		| dirdeclarator '[' typequallist '*' ']'			{/*ignore variable length array*/
										 $$=decl_append($1,decl_array_new(NULL,$3));}
		| dirdeclarator '[' '*' ']'					{$$=decl_append($1,decl_array_new(NULL,NULL));}
		| dirdeclarator '(' {scope_push(ST_PROTO);} paramtypelist {scope_pop();} ')'
										{$$=decl_append($1,decl_function_new($4));}
		| dirdeclarator '(' identlist ')'				{/*reject old C function syntax*/
										 yyerror_fatal("old function declaration style not allowed");}
		| dirdeclarator '(' {scope_push(ST_PROTO);scope_pop();} ')'	{$$=decl_append($1,decl_function_new(NULL));}
		;

pointer:	'*' typequallist						{$$=decl_pointer_new($2);}
		| '*'								{$$=decl_pointer_new(NULL);}
		| '*' typequallist pointer					{$$=decl_pointer_new($2);LL_NEXT_OF($$)=$3;}
		| '*' pointer							{$$=decl_pointer_new(NULL);LL_NEXT_OF($$)=$2;}
		;

typequallist:	typequal							{ALLOC_TYPE($$,NT_DECLSPEC);$$->declspec.tq=$1;}
		| typequallist typequal						{ALLOC_TYPE($$,NT_DECLSPEC);$$->declspec.tq=$2;
										 $$=merge_declspec($1,$$);}
		;

paramtypelist:	paramlist							{$$=$1;}
		| paramlist ',' ELLIPSIS					{$$=$1;LL_APPEND($1,ELLIPSIS_DECLARATOR);}
		;

paramlist:	paramdecl							{$$=$1;}
		| paramlist ',' paramdecl					{$$=$1;LL_APPEND($1,$3);}
		;

paramdecl:	declspeclist declarator						{$$=$2;decl_install($2,$1);}
		| declspeclist absdeclarator					{$$=$2;decl_finalize($2,$1);}
		| declspeclist 							{$$=decl_new(NULL);decl_finalize($$,$1);}
		;

identlist:	IDENT								{/*NOT DEALING WITH THIS*/}
		| identlist ',' IDENT						{/*ONLY FOR OLD C FN SYNTAX*/}
		;

/* 6.7.6: type names */
typename:	specquallist absdeclarator					{$$=$2;decl_finalize($$,$1);}
		| specquallist							{$$=decl_new(NULL);decl_finalize($$,$1);}
		;

absdeclarator:	pointer								{$$=decl_new(NULL);decl_append($$,$1);}
		| pointer dirabsdeclarator					{$$=decl_append($2,$1);}
		| dirabsdeclarator						{$$=$1;}
		;

dirabsdeclarator: '(' absdeclarator ')'						{$$=$2;}
		| dirabsdeclarator '[' typequallist asnmtexpr ']'		{$$=decl_append($1,decl_array_new($4,$3));}
		| dirabsdeclarator '[' asnmtexpr ']'				{$$=decl_append($1,decl_array_new($3,NULL));}
		| dirabsdeclarator '[' typequallist ']'				{$$=decl_append($1,decl_array_new(NULL,$3));}
		| dirabsdeclarator '[' ']'					{$$=decl_append($1,decl_array_new(NULL,NULL));}
		| '[' typequallist asnmtexpr ']'				{$$=decl_new(NULL);decl_append($$,decl_array_new($3,$2));}
		| '[' asnmtexpr ']'						{$$=decl_new(NULL);decl_append($$,decl_array_new($2,NULL));}
		| '[' typequallist ']'						{$$=decl_new(NULL);decl_append($$,decl_array_new(NULL,$2));}
		| '[' ']'							{$$=decl_new(NULL);decl_append($$,decl_array_new(NULL,NULL));}
		| dirabsdeclarator '[' STATIC typequallist asnmtexpr ']'	{/*ignore static keyword*/
										 $$=decl_append($1,decl_array_new($5,$4));}
		| dirabsdeclarator '[' STATIC asnmtexpr ']'			{$$=decl_append($1,decl_array_new($4,NULL));}
		| dirabsdeclarator '[' STATIC typequallist ']'			{$$=decl_append($1,decl_array_new(NULL,$4));}
		| dirabsdeclarator '[' STATIC ']'				{$$=decl_append($1,decl_array_new(NULL,NULL));}
		| '[' STATIC typequallist asnmtexpr ']'				{$$=decl_new(NULL);decl_append($$,decl_array_new($4,$3));}
		| '[' STATIC asnmtexpr ']'					{$$=decl_new(NULL);decl_append($$,decl_array_new($3,NULL));}
		| '[' STATIC typequallist ']'					{$$=decl_new(NULL);decl_append($$,decl_array_new(NULL,$3));}
		| '[' STATIC ']'						{$$=decl_new(NULL);decl_append($$,decl_array_new(NULL,NULL));}
		| dirabsdeclarator '[' typequallist STATIC asnmtexpr ']'	{$$=decl_new(NULL);decl_append($$,decl_array_new($5,$3));}
		| '[' typequallist STATIC asnmtexpr ']'				{$$=decl_new(NULL);decl_append($$,decl_array_new($4,$2));}
		| dirabsdeclarator '[' '*' ']'					{/*ignore/throw error on VLA*/
										 yyerror("VLAs not supported");}
		| '[' '*' ']'							{yyerror("VLAs not supported");}
		| dirabsdeclarator '(' paramtypelistopt ')'			{$$=decl_append($1,decl_function_new($3));}
		| '(' paramtypelistopt ')'					{$$=decl_new(NULL);decl_append($$,decl_function_new($2));}
		;

paramtypelistopt:	paramtypelist						{$$=$1;}
		|								{$$=NULL;}
		;

/* 6.7.7 type definitions */
/*typedefname:	IDENT								{/*not implementing typedefs}*/
/*		;*/

/* 6.8 statements and blocks */
stmt:		labeledstmt							{$$=$1;}
		| compoundstmt							{$$=$1;}
		| exprstmt							{$$=$1;}
		| selectionstmt							{$$=$1;}
		| iterationstmt							{$$=$1;}
		| jumpstmt							{$$=$1;}
		;

/* 6.8.1 labeled statements */
labeledstmt:	IDENT ':' stmt							{ALLOC_STMT_LABEL($$, LABEL_NAMED, $1, NULL, $3);}
		| CASE condexpr ':' stmt					{ALLOC_STMT_LABEL($$, LABEL_CASE,  NULL, $2, $4);}
		| DEFAULT ':' stmt						{ALLOC_STMT_LABEL($$, LABEL_DEFAULT,  NULL, NULL, $3);}
		;

/* 6.8.2 compound statement */
compoundstmt:	'{' {scope_push(0);} blockitemlist {scope_pop();} '}'		{/*have to be wary that since next token is seen by the time of the midrule action*/
										 ALLOC_STMT_COMPOUND($$,$3);}
		| '{' {scope_push(0);scope_pop();} '}'				{ALLOC_STMT_COMPOUND($$,NULL);}
		;

blockitemlist:	blockitem							{$$=$1;}
		| blockitemlist blockitem					{if(!$1) {$$=$2;}
										 else if(!$2) {$$=$1;}
										 else {$$=$1;LL_APPEND($1,$2);}}
		;

blockitem:	decl								{/*don't need to do anything*/}
		| stmt								{$$=$1;}
		;

/* 6.8.3 expression and null statements */
exprstmt:	expr ';'							{ALLOC_STMT_EXPR($$,$1);}
		| ';'								{$$=NULL;}
		;

/* 6.8.4 selection statements */
selectionstmt:	IF '(' expr ')' stmt %prec IF					{ALLOC_STMT_IFELSE($$, $3, $5, NULL);}
		| IF '(' expr ')' stmt ELSE stmt %prec ELSE			{ALLOC_STMT_IFELSE($$, $3, $5, $7);}
		| SWITCH '(' expr ')' stmt					{ALLOC_STMT_SWITCH($$, $3, $5);}
		;

/* 6.8.5 Iteration statements */
iterationstmt:	WHILE '(' expr ')' stmt						{ALLOC_STMT_WHILE($$, $3, $5);}
		| DO stmt WHILE '(' expr ')'					{ALLOC_STMT_DO_WHILE($$, $5, $2);}
		| FOR '(' expr ';' expr ';' expr ')' stmt			{ALLOC_STMT_FOR($$, $3, $5, $7, $9);}
		| FOR '(' ';' expr ';' expr ')' stmt				{ALLOC_STMT_FOR($$, NULL, $4, $6, $8);}
		| FOR '(' expr ';' ';' expr ')' stmt				{ALLOC_STMT_FOR($$, $3, NULL, $6, $8);}
		| FOR '(' expr ';' expr ';' ')' stmt				{ALLOC_STMT_FOR($$, $3, $5, NULL, $8);}
		| FOR '(' ';' ';' expr ')' stmt					{ALLOC_STMT_FOR($$, NULL, NULL, $5, $7);}
		| FOR '(' expr ';'  ';' ')' stmt				{ALLOC_STMT_FOR($$, $3, NULL, NULL, $7);}
		| FOR '(' ';' expr ';' ')' stmt					{ALLOC_STMT_FOR($$, NULL, $4, NULL, $7);}
		| FOR '(' ';' ';' ')' stmt					{ALLOC_STMT_FOR($$, NULL, NULL, NULL, $6);}
		| FOR '(' {scope_push(0);} decl fordecl ')' stmt		{$$=$5;$$->stmt_for.init=$4;$$->stmt_for.body=$7;
										 scope_pop();}
		;

/* have to add this nonterminal to get rid of shift-reduce conflicts*/
fordecl:	expr ';' expr							{ALLOC_STMT_FOR($$,NULL,$1,$3,NULL);}
		| ';' expr							{ALLOC_STMT_FOR($$,NULL,NULL,$2,NULL);}
		| expr ';'							{ALLOC_STMT_FOR($$,NULL,$1,NULL,NULL);}
		| ';'								{ALLOC_STMT_FOR($$,NULL,NULL,NULL,NULL);}
		;

/* 6.8.6 Jump Statements */
jumpstmt:	GOTO IDENT ';'							{ALLOC_STMT_GOTO($$, $2);}
		| CONTINUE ';'							{ALLOC_STMT_BREAK_CONT($$, NT_STMT_CONT);}
		| BREAK ';'							{ALLOC_STMT_BREAK_CONT($$, NT_STMT_BREAK);}
		| RETURN expr ';'						{ALLOC_STMT_RETURN($$, $2);}
		| RETURN ';'							{ALLOC_STMT_RETURN($$, NULL);}
		;

/* 6.9 External Definitions */
externdecl: 	funcdef								{$$=$1;}
		| decl								{$$=$1;}
		;

/* 6.9.1 Function definitions */
funcdef:	declspeclist declarator {decl_check_fndef($2);scope_set_fndef();decl_install($2,$1);} compoundstmt
										{/*note that this doesn't allow for old fndef syntax*/
										 $$=$2;$2->decl.fn_body=$4;
										 /*print function body*/
										 print_astnode($$);
										 /*generate quads for this function*/
										 generate_quads($$);}
		;

%%