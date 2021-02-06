#ifndef PARSERH
#define PARSERH

#include "numutils.h"
#include "stringutils.h"

enum astnode_type {
	// terminals
	NT_NUMBER,
	NT_KEYWORD,
	NT_OPERATOR,
	NT_STRING,
	NT_CHARLIT,

	// nonterminals
	NT_BINOP,
	NT_UNOP,
};

extern int yylex();
int yyerror(char *err);

struct astnode_binop {
	enum astnode_type type;
	int operator;
	union astnode *left, *right;
};

struct astnode_unop {
	enum astnode_type type;
	int operator;
	union astnode *arg;
};

struct astnode_number {
	enum astnode_type type;
	struct number num;
};

union astnode {
	// generic acts as interface assuming proper alignment
	struct astnode_generic {int type;} generic;

	struct astnode_number num;
	struct astnode_binop binop;
	struct astnode_unop unop;
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
	(var)->binop=(struct astnode_binop){NT_BINOP, op, left, right}

#define ALLOC_SET_UNOP(var, op, arg)\
	ALLOC(var);\
	(var)->unop=(struct astnode_unop){NT_UNOP, op, arg};

// helper to indent to a specific depth
#define INDENT(n)\
	for (int indi = 0; indi < depth; indi++) fprintf(stdout, "\t");

#endif