#include "astnode.h"

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
	ST_TYPEDEF } st;

struct attnode_typespec_scalar {
	enum spec_type type;
	int scalartype;	// using lexer/parser enum constants
	struct attnode_typespec_scalar *next;
};

struct attnode_typespec_fn {
	enum spec_type type;
	union attnode_typespec *rettype;

	// TODO: how to deal with param names? right now using full
	// declarations to describe parameters
	union attnode *param_list;
};

struct attnode_typespec_array {
	enum spec_type type;
	unsigned len;
	union attnode_typespec *arrtype;
};

// TODO: structs and unions

struct attnode_typespec_struct_union {
    union attnode *member;
    int tag_define;
    enum spec_type type;

};

union attnode_typespec {
	struct attnode_typespec_generic {
		enum spec_type type;
	} generic;
	struct attnode_typespec_scalar scalar;
	struct attnode_typespec_fn fn;
	struct attnode_typespec_array array;
};

struct attnode_typequal {
	enum type_qual { TQ_CONST, TQ_RESTRICT, TQ_VOLATILE } qual;
	struct attnode_typequal *next;
};

struct attnode_storagespec {	
	enum storagespec { SS_EXTERN, SS_STATIC, SS_AUTO, SS_REGISTER } spec;
	struct attnode_storagespec *next;
};

struct attnode_varfn {
	char *ident;
	union attnode_typespec *ts;
	struct attnode_typequal *tq;
	struct attnode_storagespec *ss;
	struct attnode_varfn *next;
	union astnode *initializer;
};

struct attnode_label {
	char *ident;
};

union attnode {
	struct attnode_varfn vf;
	struct attnode_label label;
};