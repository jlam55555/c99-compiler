#ifndef ASTNODEH
#define ASTNODEH

#include <stdlib.h>	// for malloc in macro
#include <string.h>	// for strdup in macro
#include "lexerutils/numutils.h"
#include "lexerutils/stringutils.h"
#include "asttypes.h"
#include "astnodegeneric.h"

struct astnode_binop {
	_ASTNODE

	int op;
	union astnode *left, *right;
};

struct astnode_unop {
	_ASTNODE

	int op;
	union astnode *arg;
};

struct astnode_ternop {
	_ASTNODE

	union astnode *first, *second, *third;
};

struct astnode_number {
	_ASTNODE

	struct number num;
};

struct astnode_ident {
	_ASTNODE

	char *ident;
};

struct astnode_charlit {
	_ASTNODE

	struct charlit charlit;
};

struct astnode_string {
	_ASTNODE

	struct string string;
};

struct astnode_fncall {		// function invocation
	_ASTNODE

	union astnode *fnname, *arglist;
};

struct astnode_comlit {		// compound literal
	_ASTNODE

	// typename is keyword in C++ but not C so this is ok
	union astnode *typename, *initlist;
};

union astnode {
	// generic acts as interface assuming proper alignment
	struct astnode_generic generic;
	
	// expr types
	struct astnode_number num;
	struct astnode_binop binop;
	struct astnode_unop unop;
	struct astnode_ternop ternop;
	struct astnode_ident ident;
	struct astnode_charlit charlit;
	struct astnode_string string;
	struct astnode_fncall fncall;
	struct astnode_comlit comlit;

	// decl types
	struct astnode_typespec_scalar ts_scalar;
	struct astnode_typespec_fn ts_fn;
	struct astnode_typespec_array ts_array;
	struct astnode_typespec_struct_union ts_struct_union;
	struct astnode_typequal tq;
	struct astnode_storageclass sc;
	struct astnode_declspec declspec;
	struct astnode_varfn varfn;
	struct astnode_label label;

	// declarator types
	struct astnode_pointer ptr;
	struct astnode_declarator declarator;
	struct astnode_dirdeclarator dirdeclarator;

};

// helper to print an astnode
void print_astnode(union astnode *);

#define ALLOC_SET_IDENT(var, idt)\
	ALLOC(var);\
	(var)->ident=(struct astnode_ident){NT_IDENT, NULL, NULL, strdup(idt)}

#define ALLOC_SET_BINOP(var, op, left, right)\
	ALLOC(var);\
	(var)->binop=(struct astnode_binop){NT_BINOP, NULL, NULL, op, left, right}

#define ALLOC_SET_UNOP(var, op, arg)\
	ALLOC(var);\
	(var)->unop=(struct astnode_unop){NT_UNOP, NULL, NULL, op, arg};

#define ALLOC_SET_TERNOP(var, first, second, third)\
	ALLOC(var);\
	(var)->ternop=(struct astnode_ternop){NT_TERNOP, NULL, NULL, first, second,third};

// rewrite assignment-equals operators
#define ASNEQ(var, type, left, right) \
	union astnode *inner; \
	ALLOC_SET_BINOP(inner, type, left, right); \
	ALLOC_SET_BINOP(var, '=', left, inner)

#endif