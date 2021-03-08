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
};

#define _ASTNODE\
	enum astnode_type type;\
	union astnode *prev, *next;

struct astnode_generic {
	_ASTNODE
};

#endif