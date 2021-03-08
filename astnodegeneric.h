#ifndef ASTNODEGENERICH
#define ASTNODEGENERICH

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
	NT_VARFN,
	NT_LABEL,
	
	// declarator types
	NT_DECLARATOR,
	NT_DIRDECLARATOR,
	NT_PTR

};

#define _ASTNODE\
	enum astnode_type type;\
	union astnode *prev, *next;

// this is declared in asttypes.c
extern union astnode *ll_append_iter;
#define LL_APPEND(ll, node)\
	ll_append_iter = node->generic;\
	while (ll_append_iter->generic.next) {\
		ll_append_iter = ll_append_iter->generic.next\
	}\
	ll_append_iter->generic.next = node;

struct astnode_generic {
	_ASTNODE
};

#endif