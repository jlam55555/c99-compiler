#ifndef PARSERH
#define PARSERH

#include "numutils.h"
#include "stringutils.h"

enum astnode_type {
	// terminals
	NT_NUMBER,
	NT_KEYWORD,
	NT_STRING,
	NT_CHARLIT,
	NT_IDENT,

	// nonterminals
	NT_BINOP,
	NT_UNOP,
	NT_FNCALL,	// function invocation
	NT_COMLIT,	// compound literal
};

extern int yylex();
int yyerror(char *err);

struct astnode_generic {
	int type;
	union astnode *prev, *next;
};

struct astnode_binop {
	enum astnode_type type;
	union astnode *prev, *next;
	int op;
	union astnode *left, *right;
};

struct astnode_unop {
	enum astnode_type type;
	union astnode *prev, *next;
	int op;
	union astnode *arg;
};

struct astnode_number {
	enum astnode_type type;
	union astnode *prev, *next;
	struct number num;
};

struct astnode_ident {
	enum astnode_type type;
	union astnode *prev, *next;
	char *ident;
};

struct astnode_charlit {
	enum astnode_type type;
	union astnode *prev, *next;
	struct charlit c;
};

struct astnode_string {
	enum astnode_type type;
	union astnode *prev, *next;
	struct string s;
};

struct astnode_fncall {		// function invocation
	enum astnode_type type;
	union astnode *prev, *next;
	union astnode *fnname, *arglist;
};

struct astnode_comlit {		// compound literal
	enum astnode_type type;
	union astnode *prev, *next;
	// typename is keyword in C++ but not C so this is ok
	union astnode *typename, *initlist;
};

union astnode {
	// generic acts as interface assuming proper alignment
	struct astnode_generic generic;
	
	struct astnode_number num;
	struct astnode_binop binop;
	struct astnode_unop unop;
	struct astnode_ident ident;
	struct astnode_charlit charlit;
	struct astnode_string string;
	struct astnode_fncall fncall;
	struct astnode_comlit comlit;
};

// helper to allocate an astnode
union astnode *astnode_alloc();

// helper to print an astnode
void print_astnode(union astnode *);

// helpers to allocating and initialize new AST nodes
#define ALLOC(var)\
	var = astnode_alloc();

#define ALLOC_SET_BINOP(var, op, left, right)\
	ALLOC(var);\
	(var)->binop=(struct astnode_binop){NT_BINOP, NULL, NULL, op, left, right}

#define ALLOC_SET_UNOP(var, op, arg)\
	ALLOC(var);\
	(var)->unop=(struct astnode_unop){NT_UNOP, NULL, NULL, op, arg};

// helper to indent to a specific depth
#define INDENT(n)\
	for (int indi = 0; indi < depth; indi++) fprintf(stdout, "\t");

#endif