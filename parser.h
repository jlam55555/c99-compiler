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

union astnode *astnode_alloc();

void print_astnode(union astnode *);

#endif