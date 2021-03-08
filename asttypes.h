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

	enum spec_type spectype;
	int scalartype;	// using lexer/parser enum constants
	struct modifiers {
		char ll: 2;
		char sign: 1;
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

	struct {
		char qual_const: 1;
		char qual_restrict: 1;
		char qual_volatile: 1;
	} qual;
};

struct astnode_storagespec {	
	_ASTNODE

	enum sc_spec { SC_EXTERN, SC_STATIC, SC_AUTO, SC_REGISTER } scspec;
};

struct astnode_declspec {
	_ASTNODE
	
	union astnode *ts, *tq, *ss;
};

struct astnode_varfn {
	_ASTNODE

	char *ident;
	union astnode *declspec;

	// TODO: ignore
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

// helpers to alloc att
#define AST_ALLOC(var)\
	var = malloc(sizeof(union astnode));

#define ALLOC_DECLSPEC(var)\
	var = malloc(sizeof(struct astnode_declspec));

#define ALLOC_SET_SCSPEC(var, sc)\
	var = malloc(sizeof(struct astnode_storagespec));\
	(var)->type = NT_SC;\
	(var)->scspec = sc;

#define AST_ALLOC_SET_SCALAR(var, scalartype, ll, sign)\
	var = malloc(sizeof(struct astnode_typespec_scalar));\
	(var)->spectype

#define AST_ALLOC_SET_FLOAT(var, scalartype, ll, sign)\
	ALLOC(var);\
	(var)->vf->ts->scalar.type= ST_FLOAT;\
	(var)->vf->ts->scalar.scalartype = scalartype;\
	(var)->vf->ts->scalar.modifiers.ll = ll;\
	(var)->vf->ts->scalar.modifiers.sign = sign;

#endif // ATTNODEH