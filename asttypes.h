#ifndef ASTTYPESH
#define ASTTYPESH

#include "astnode.h"
#include "astnodegeneric.h"

/*
each entry in symbol table (for each declaration or label):
	- one of: label, variable/function
	- identifier
	- for variables/functions:
		- type spec
		- type qualifier list
		- storage specifier classifier

type spec:
	- one of: function, variable, tag, pointer, array
	- (for function): return type and parameter list
	- (for scalar variable): type
	- (for compound variable): union/struct spec
	- (for enum): ???
	- (for pointer): what it points to
	- (for array): length, array type

type qualifier:
	- enum type_qual

storage spec:
	- enum storage_spec
*/
enum spec_type { ST_SCALAR, ST_FN, ST_TAG, ST_POINTER, ST_ARRAY,
	ST_TYPEDEF} st;

#define TS_SCALAR_SIGNED	0x4
#define TS_SCALAR_LL		0x2
#define TS_SCALAR_L		0x1
struct astnode_typespec_scalar {
	_ASTNODE

	// TODO: implement _Complex (maybe as modifier?)
	enum spec_type spectype;
	enum scalar_basetype {BT_UNSPEC, BT_CHAR, BT_VOID, BT_INT,
		BT_DOUBLE, BT_FLOAT, BT_BOOL}
		basetype;	// using lexer/parser enum constants
	struct modifiers {
		enum {LLS_UNSPEC, LLS_LONG, LLS_LONG_LONG, LLS_SHORT} lls;
		enum {SIGN_UNSPEC, SIGN_SIGNED, SIGN_UNSIGNED} sign;
	} modifiers;
};

struct astnode_typespec_fn {
	_ASTNODE

	enum spec_type spectype;
	union astnode_typespec *rettype;

	// TODO: how to deal with param names? right now using full
	// declarations to describe parameters
	union astnode *param_list;
};

struct astnode_typespec_array {
	_ASTNODE

	enum spec_type spectype;
	unsigned len;
	union astnode_typespec *arrtype;
};

// TODO: structs and unions

struct astnode_typespec_struct_union {
	_ASTNODE

	enum spec_type spectype;
	union astnode *member;
	int tag_define;
};

// TODO: remove
// union astnode_typespec {
// 	struct astnode_typespec_generic {
// 		_ASTNODE
// 		enum spec_type spectype;
// 	} generic;
// 	struct astnode_typespec_scalar scalar;
// 	struct astnode_typespec_fn fn;
// 	struct astnode_typespec_array array;
// };

#define TQ_CONST	0x1
#define TQ_RESTRICT	0x2
#define TQ_VOLATILE	0x4
struct astnode_typequal {
	_ASTNODE

	unsigned char qual;
};

struct astnode_storageclass {	
	_ASTNODE

	enum sc_spec { SC_EXTERN, SC_STATIC, SC_AUTO, SC_REGISTER } scspec;
};

struct astnode_declspec {
	_ASTNODE
	
	union astnode *ts, *tq, *sc;
};

struct astnode_varfn {
	_ASTNODE

	char *ident;
	union astnode *declspec;

	// TODO: remove; replaced with declspec
	// union astnode_typespec *ts;
	// struct astnode_typequal *tq;
	// struct astnode_storagespec *ss;
	// struct astnode_varfn *next;
	// union astnode *initializer;
};

struct astnode_label {
	_ASTNODE

	char *ident;
};

// declarators
struct astnode_pointer {
	_ASTNODE

	union astnode *typequallist, *to;
};

struct astnode_declarator {
	_ASTNODE

	union astnode *pointer, *dirdeclarator;
};

struct astnode_dirdeclarator {
	_ASTNODE

	enum declarator_type {DT_REGULAR, DT_ARRAY, DT_FN} declarator_type;
	int is_abstract;

	// this can be either an NT_IDENT or NT_DIRDECLARATOR -- check type
	// before using it
	union astnode *ident;

	// if function
	union astnode *paramtypelist;

	// if array (assume size is an integer literal for now)
	union astnode *size;

	// when in function declarator; these will be moved to the pointer
	union astnode *typequallist;

	// NOTE: we are not doing anything with the STATIC keyword
};

extern const struct astnode_dirdeclarator ELLIPSIS_DECLARATOR;

// helpers to alloc att
#define AST_ALLOC(var)\
	var = malloc(sizeof(union astnode));

#define ALLOC_DECLSPEC(var)\
	var = malloc(sizeof(struct astnode_declspec));

#define ALLOC_SET_SCSPEC(var, sc)\
	var = malloc(sizeof(struct astnode_storageclass));\
	(var)->type = NT_SC;\
	(var)->scspec = sc;

#define ALLOC_SET_TQSPEC(var, tq)\
	var = malloc(sizeof(struct astnode_typequal));\
	(var)->type = NT_TQ;\
	(var)->qual |= tq;

#define ALLOC_SET_SCALAR(var, scalartype, lls, sign)\
	var = malloc(sizeof(struct astnode_typespec_scalar));\
	(var)->type = NT_TS_SCALR;\
	(var)->spectype = ST_SCALAR;\
	(var)->basetype = scalartype;\
	(var)->modifiers->lls = lls;

#define ALLOC_DECLARATOR(var, dirdeclarator, ptr)


#define ALLOC_REGULAR_DIRDECLARATOR(var, ident)\
	var = malloc(sizeof(struct astnode_dirdeclarator));\
	(var)->type = NT_DIRDECLARATOR;\
	(var)->ident = ident;


#define ALLOC_ARRAY_DIRDECLARATOR(var, dirdeclarator, typequalist, asnmntexpr)\
	var = malloc(sizeof(struct astnode_dirdeclarator));\
	(var)->type = NT_DIRDECLARATOR;\

#define ALLOC_FN_DIRDECLARATOR(var, dirdeclarator, paramtypelist)\
	var = malloc(sizeof(struct astnode_dirdeclarator));\
	(var)->type = NT_DIRDECLARATOR;\

#endif // ATTNODEH