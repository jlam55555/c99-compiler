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

#define TS_SCALAR_SIGNED	0x4
#define TS_SCALAR_LL		0x2
#define TS_SCALAR_L		0x1
struct attnode_typespec_scalar {
	enum spec_type type;
	int scalartype;	// using lexer/parser enum constants
	struct modifiers {
		char ll: 2;
		char sign: 1;
	} modifiers;
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

#define TQ_CONST	0x1
#define TQ_RESTRICT	0x2
#define TQ_VOLATILE	0x4
struct attnode_typequal {
	struct {
		char qual_const: 1;
		char qual_restrict: 1;
		char qual_volatile: 1;
	} qual;
	struct attnode_typequal *next;
};

#define SS_EXTERN	0x1
#define SS_STATIC	0x2
#define SS_AUTO		0x4
#define SS_REGISTER	0x8
struct attnode_storagespec {	
	struct {
		char spec_extern: 1;
		char spec_static: 1;
		char spec_auto: 1;
		char spec_register: 1;
	} spec;
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