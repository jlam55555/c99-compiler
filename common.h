/**
 * 	This file (and the accompanying .c file) is used to declare and
 * 	initialize values that may be used throughout the application. This file
 * 	has no dependencies, i.e., it can be included anywhere in the
 * 	application.
 *
 * 	i.e., miscellaneous global variables for macros and such
 */

#ifndef COMMONH
#define COMMONH

enum astnode_type {
	// terminals
	NT_NUMBER,
	NT_STRING,
	NT_CHARLIT,
	NT_IDENT,

	// nonterminals
	NT_BINOP,
	NT_UNOP,
	NT_TERNOP,
	NT_FNCALL,	// function invocation
	NT_COMLIT,	// compound literal

	// declaration types
	NT_TS_SCALAR,
	NT_TS_FN,
	NT_TS_ARRAY,
	NT_TS_STRUCT_UNION,
	NT_TQ,
	NT_SC,

	// declarator types
	NT_DECLSPEC,
	NT_DECLARATOR_POINTER,
	NT_DECLARATOR_FUNCTION,
	NT_DECLARATOR_ARRAY,
	NT_DECLARATION,

	// symbol table types
	NT_SYMBOL
};

// any astnode union type can be used as a linked list without a container type
#define _ASTNODE\
	enum astnode_type type;\
	union astnode *next;

// "interface" for astnodes
struct astnode_generic {
	_ASTNODE
};

// assumes ll is not null
extern union astnode *ll_append_iter;
#define LL_APPEND(ll, node) {\
	ll_append_iter = ll;\
	while (ll_append_iter->generic.next) {\
		ll_append_iter = ll_append_iter->generic.next;\
	}\
	ll_append_iter->generic.next = node;\
}

// get next element in a linked list
#define LL_NEXT(ll) ((ll)->generic.next)

// global symbol to represent ellipsis declarator as an ordinary union astnode *
// so that we don't have to handle it separately
extern union astnode *ELLIPSIS_DECLARATOR;

// helper to indent to a specific depth
extern int indi;
#define INDENT(n)\
	for (indi = 0; indi < n; indi++) \
		fprintf(stdout, "  ");

#endif	// COMMONH