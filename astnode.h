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
	struct astnode_paramdecl paramdecl;
	struct astnode_typename typename;

	// symbol table
	struct astnode_symbol symbol;
};

// helper to print an astnode
void print_astnode(union astnode *);

#define ALLOC(var)\
	(var)=(union astnode *)malloc(sizeof(union astnode));

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

// helpers to alloc att
#define ALLOC_DECLSPEC(var)\
	ALLOC(var);\
	(var)->declspec.type = NT_DECLSPEC;\


#define ALLOC_SET_SCSPEC(var, storageclass)\
	ALLOC(var);\
	(var)->sc.type = NT_SC;\
	(var)->sc.scspec = storageclass;

#define ALLOC_SET_TQSPEC(var, typequal)\
	ALLOC(var);\
	(var)->tq.type = NT_TQ;\
	(var)->tq.qual |= typequal;

#define ALLOC_SET_SCALAR(var, scalartype, longlongshort, signunsign)\
	ALLOC(var);\
	(var)->ts_scalar.type = NT_TS_SCALAR;\
	(var)->ts_scalar.spectype = ST_SCALAR;\
	(var)->ts_scalar.basetype = scalartype;\
	(var)->ts_scalar.modifiers.lls = longlongshort;\
	(var)->ts_scalar.modifiers.sign = signunsign;

#define ALLOC_DECLARATOR(var, dd, ptr, abs)\
	ALLOC(var);\
	(var)->declarator.type = NT_DECLARATOR;\
	(var)->declarator.dirdeclarator = dd;\
	(var)->declarator.is_abstract = abs;\
	(var)->declarator.pointer = ptr;

/*
int **const *x;

DECLSPEC * * TYPEQUAL POINTER
DECLSPEC * POINTER
DECLSPEC POINTER

struct typespec (int)

1 struct pointer -> v
2 struct pointer -> v
3 struct pointer -> int

struct directdeclarator
*/

#define ALLOC_POINTER(var, tqlist, from)\
	ALLOC(var);\
	(var)->ptr.type = NT_POINTER;\
	(var)->ptr.typequallist = tqlist;\
	if (from) {\
		union astnode *iter = (from);\
		while (iter->ptr.to) {\
			iter = iter->ptr.to;\
		}\
		iter->ptr.to = (var);\
	}

#define ALLOC_PARAMDECL(var, ds, declr)\
	ALLOC(var);\
	(var)->paramdecl.type = NT_PARAMDECLARATOR;\
	(var)->paramdecl.declspec = ds;\
	(var)->paramdecl.declarator = declr;

#define ALLOC_TYPENAME(var, sql, declr)\
	ALLOC(var);\
	(var)->typename.sql = specquallist;\
	(var)->typename.declarator = declr;

#define ALLOC_REGULAR_DIRDECLARATOR(var, idt)\
	ALLOC(var);\
	(var)->dirdeclarator.type = NT_DIRDECLARATOR;\
	(var)->dirdeclarator.ident = idt;\
	(var)->dirdeclarator.is_abstract = 0;

#define ALLOC_ARRAY_DIRDECLARATOR(var, dd, tql, sizeexpr, abs)\
	ALLOC(var);\
	(var)->dirdeclarator.type = NT_DIRDECLARATOR;\
	(var)->dirdeclarator.ident = dd;\
	(var)->dirdeclarator.typequallist = tql;\
	(var)->dirdeclarator.size = sizeexpr;\
	(var)->dirdeclarator.is_abstract = abs;

#define ALLOC_FN_DIRDECLARATOR(var, dd, ptl, abs)\
	ALLOC(var);\
	(var)->dirdeclarator.type = NT_DIRDECLARATOR;\
	(var)->dirdeclarator.ident = dd;\
	(var)->dirdeclarator.paramtypelist = ptl;\
	(var)->dirdeclarator.is_abstract = abs;

#endif