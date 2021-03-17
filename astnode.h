#ifndef ASTNODEH
#define ASTNODEH

#include <stdlib.h>	// for malloc in macro
#include <string.h>	// for strdup in macro
#include "lexerutils/numutils.h"
#include "lexerutils/stringutils.h"
#include "asttypes.h"
#include "astnodegeneric.h"
#include "declarator.h"

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
//	struct astnode_typespec_fn ts_fn;
//	struct astnode_typespec_array ts_array;
	struct astnode_typespec_structunion ts_structunion;
	struct astnode_typequal tq;
	struct astnode_storageclass sc;
	struct astnode_declspec declspec;

	// declarator types
	struct astnode_declarator_generic declarator_component;
	struct astnode_declarator_pointer pointer;
	struct astnode_declarator_function fn;
	struct astnode_declarator_array array;
	struct astnode_declarator declarator;
	struct astnode_declaration declaration;
//	struct astnode_pointer ptr;
//	struct astnode_declarator declarator;
//	struct astnode_dirdeclarator dirdeclarator;
//	struct astnode_paramdecl paramdecl;
//	struct astnode_typename typename;

	// symbol table
//	struct astnode_variable variable;
	struct astnode_symbol symbol;
};

// helper to print an astnode
void print_astnode(union astnode *);

// use calloc because it also zeros (malloc is not guaranteed to zero)
#define ALLOC(var)\
	(var)=(union astnode *)calloc(1, sizeof(union astnode));

#define ALLOC_TYPE(var, type_name)\
	ALLOC(var);\
	(var)->generic.type = type_name;

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
	(var)->declspec.type = NT_DECLSPEC;

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
	(var)->typename.specquallist = sql;\
	(var)->typename.absdeclarator = declr;

#define ALLOC_REGULAR_DIRDECLARATOR(var, idt)\
	ALLOC(var);\
	(var)->dirdeclarator.type = NT_DIRDECLARATOR;\
	(var)->dirdeclarator.declarator_type = DT_REGULAR;\
	(var)->dirdeclarator.ident = idt;\
	(var)->dirdeclarator.is_abstract = 0;

#define ALLOC_ARRAY_DIRDECLARATOR(var, dd, tql, sizeexpr, abs)\
	ALLOC(var);\
	(var)->dirdeclarator.type = NT_DIRDECLARATOR;\
	(var)->dirdeclarator.is_abstract = abs;\
	union astnode *idt, *node = (dd), *iter;\
	if (abs && node==NULL) {\
		ALLOC_SET_IDENT(idt, "");\
		ALLOC_REGULAR_DIRDECLARATOR(node, idt);\
		node->dirdeclarator.is_abstract = 1;\
	}\
	/*loop to innermost dirdeclarator*/\
	/*node is top-level, iter is inner-most, var gets added to innermost*/\
	iter = node;\
	while (iter->dirdeclarator.ident->generic.type == NT_DIRDECLARATOR) {\
		iter = iter->dirdeclarator.ident;\
	}\
	iter->dirdeclarator.declarator_type = DT_ARRAY;\
	iter->dirdeclarator.typequallist = tql;\
	iter->dirdeclarator.size = sizeexpr;\
	(var)->dirdeclarator.ident = iter->dirdeclarator.ident;\
	iter->dirdeclarator.ident = (var);\
	/*return the top dirdeclarator*/\
	(var) = node;

#define ALLOC_FN_DIRDECLARATOR(var, dd, ptl, abs)\
	ALLOC(var);\
	(var)->dirdeclarator.type = NT_DIRDECLARATOR;\
	(var)->dirdeclarator.declarator_type = DT_FN;\
	(var)->dirdeclarator.ident = dd;\
	(var)->dirdeclarator.paramtypelist = ptl;\
	(var)->dirdeclarator.is_abstract = abs;

#endif