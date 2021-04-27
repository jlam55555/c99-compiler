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

// basic debugging info -- like the level in Hak's output
#define DEBUG 1

// extra debugging for debugging errors -- e.g., types for struct addr,
// quads used in generation of sizeof operand (usually not emitted)
#define DEBUG2 0

// all astnode types
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

	// declaration types
	NT_TS_SCALAR,
	NT_TS_STRUCT_UNION,
	NT_TQ,
	NT_SC,

	// declarator types
	NT_DECLSPEC,
	NT_DECLARATOR_POINTER,
	NT_DECLARATOR_FUNCTION,
	NT_DECLARATOR_ARRAY,
	NT_DECL,

	// statement types
	NT_STMT_EXPR,
	NT_STMT_LABEL,
	NT_STMT_COMPOUND,
	NT_STMT_IFELSE,
	NT_STMT_SWITCH,
	NT_STMT_DO_WHILE,
	NT_STMT_WHILE,
	NT_STMT_FOR,
	NT_STMT_GOTO,
	NT_STMT_CONT,
	NT_STMT_BREAK,
	NT_STMT_RETURN,
};

// any astnode union type can be used as a linked list without a container type
#define _ASTNODE\
	enum astnode_type type;\
	union astnode *next;

// "interface" for astnodes
struct astnode_generic {
	_ASTNODE
};

#define NT(node) (node)->generic.type

// assumes ll is not null
extern union astnode *ll_append_iter;
#define _LL_APPEND(ll, node, next) {\
	ll_append_iter = ll;\
	while (ll_append_iter->next) {\
		ll_append_iter = ll_append_iter->next;\
	}\
	ll_append_iter->next = node;\
}

// linked list iterator: need to provide iterator pointer
#define _LL_FOR(ll, iter, next)\
	for ((iter) = ll; (iter); (iter)=(iter)->next)

// get next element in a linked list
#define _LL_NEXT(ll, next) ((ll)->next)

// for convenience
#define LL_APPEND(ll, node)	_LL_APPEND(ll, node, generic.next)
#define LL_NEXT(ll) 		_LL_NEXT(ll, generic.next)
#define LL_FOR(ll, iter)	_LL_FOR(ll, iter, generic.next)

// global symbol to represent ellipsis declarator as an ordinary union astnode *
// so that we don't have to handle it separately
extern union astnode *ELLIPSIS_DECLARATOR;

// helper to indent to a specific depth
extern int indi;
#define INDENT(depth)\
	for (indi = 0; indi < (depth); indi++) \
		fprintf(stdout, "  ");

// indicates a feature is not yet implemented; somewhat equivalent to a TODO
// (not that it will not be implemented)
#define NYI(what) yyerror(#what " not yet implemented");

#endif	// COMMONH