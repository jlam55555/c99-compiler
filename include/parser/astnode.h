/**
 * 	Defining the core (expression) AST node types, the astnode union,
 * 	and several macros dealing with union astnodes.
 */

#ifndef ASTNODEH
#define ASTNODEH

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <lexer/numutils.h>
#include <lexer/stringutils.h>
#include <parser/decl.h>
#include <parser/declspec.h>
#include <parser/structunion.h>
#include <parser/stmt.h>

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

// numeric literal
struct astnode_number {
	_ASTNODE

	// struct number num;

	// store numeric typespec
	union astnode *ts;

	// store byte representation
	u_int8_t buf[16];
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
	char *label;
};

struct astnode_fncall {		// function invocation
	_ASTNODE

	union astnode *fnname, *arglist;
};

struct astnode_comlit {		// compound literal
	_ASTNODE

	// typename is keyword in C++ but not C so this is ok
	union astnode *tag, *initlist;
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
	struct astnode_typespec_structunion ts_structunion;
	struct astnode_typequal tq;
	struct astnode_storageclass sc;
	struct astnode_declspec declspec;

	// declarator types
	struct astnode_decl_component decl_component;
	struct astnode_decl_pointer decl_pointer;
	struct astnode_decl_function decl_function;
	struct astnode_decl_array decl_array;
	struct astnode_decl decl;

	// statement types
	struct astnode_stmt_expr stmt_expr;
	struct astnode_stmt_label stmt_label;
	struct astnode_stmt_compound stmt_compound;
	struct astnode_stmt_if_else stmt_if_else;
	struct astnode_stmt_switch stmt_switch;
	struct astnode_stmt_do_while stmt_do_while;
	struct astnode_stmt_while stmt_while;
	struct astnode_stmt_for stmt_for;
	struct astnode_stmt_goto stmt_goto;
	struct astnode_stmt_break_cont stmt_bc;
	struct astnode_stmt_return stmt_return;

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
	(var)->ident=(struct astnode_ident){NT_IDENT, NULL, strdup(idt)}

#define ALLOC_SET_BINOP(var, op, left, right)\
	ALLOC(var);\
	(var)->binop=(struct astnode_binop){NT_BINOP, NULL, op, left, right}

#define ALLOC_SET_UNOP(var, op, arg)\
	ALLOC(var);\
	(var)->unop=(struct astnode_unop){NT_UNOP, NULL, op, arg};

#define ALLOC_SET_TERNOP(var, first, second, third)\
	ALLOC(var);\
	(var)->ternop=(struct astnode_ternop){NT_TERNOP, NULL, first, second,third};

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
	(var)->ts_scalar.basetype = scalartype;\
	(var)->ts_scalar.modifiers.lls = longlongshort;\
	(var)->ts_scalar.modifiers.sign = signunsign;

#define ALLOC_IMPL_FN(var, idt)\
	ALLOC_TYPE(var, NT_DECL);\
	(var)->decl.ident=idt;\
	(var)->decl.is_implicit=1;\
	ALLOC_TYPE((var)->decl.components, NT_DECLARATOR_FUNCTION);

#endif