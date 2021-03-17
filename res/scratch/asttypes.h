#ifndef ASTTYPESH
#define ASTTYPESH

#include "astnodegeneric.h"
#include "symtab.h"

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


//enum spec_type { ST_SCALAR, ST_FN, ST_TAG, ST_POINTER, ST_ARRAY,
//	ST_TYPEDEF};
//
//#define TS_SCALAR_SIGNED	0x4
//#define TS_SCALAR_LL		0x2
//#define TS_SCALAR_L		0x1
//struct astnode_typespec_scalar {
//	_ASTNODE
//
//	// TODO: implement _Complex (maybe as modifier?)
//	enum spec_type spectype;
//	enum scalar_basetype {BT_UNSPEC, BT_CHAR, BT_VOID, BT_INT,
//		BT_DOUBLE, BT_FLOAT, BT_BOOL}
//		basetype;	// using lexer/parser enum constants
//	struct modifiers {
//		enum {LLS_UNSPEC, LLS_LONG, LLS_LONG_LONG, LLS_SHORT} lls;
//		enum {SIGN_UNSPEC, SIGN_SIGNED, SIGN_UNSIGNED} sign;
//	} modifiers;
//};

//struct astnode_typespec_fn {
//	_ASTNODE
//
//	enum spec_type spectype;
//	union astnode_typespec *rettype;
//
//	// TODO: how to deal with param names? right now using full
//	// declarations to describe parameters
//	union astnode *param_list;
//};
//
//struct astnode_typespec_array {
//	_ASTNODE
//
//	enum spec_type spectype;
//	unsigned len;
//	union astnode_typespec *arrtype;
//};

// see: structunion.h
//enum structunion_type { SU_STRUCT, SU_UNION };
//struct symtab {
//	union astnode **bs;
//	int size, capacity;
//};
//struct astnode_typespec_structunion {
//	_ASTNODE
//
//	char *ident;
//	enum spec_type spectype;	// = ST_TAG
//	enum structunion_type su_type;
//
//	// linked-list and hashtable of members as symbols
//	union astnode *members;
//	struct symtab members_ht;
//
//	// to prevent multiple (nested) redefinition: see structunion.h
//	int is_complete, is_being_defined;
//
//	// for debugging purposes: prints out where struct is defined
//	char *def_filename;
//	int def_lineno;
//};

//#define TQ_CONST	0x1
//#define TQ_RESTRICT	0x2
//#define TQ_VOLATILE	0x4
//struct astnode_typequal {
//	_ASTNODE
//
//	unsigned char qual;
//};
//
//struct astnode_storageclass {
//	_ASTNODE
//
//	enum sc_spec { SC_EXTERN, SC_STATIC, SC_AUTO, SC_REGISTER } scspec;
//};
//
//struct astnode_declspec {
//	_ASTNODE
//
//	union astnode *ts, *tq, *sc;
//};

// declarators
//struct astnode_pointer {
//	_ASTNODE
//
//	union astnode *typequallist, *to;
//};
//
//struct astnode_declarator {
//	_ASTNODE
//
//	union astnode *pointer, *dirdeclarator;
//	int is_abstract;
//};
//
//struct astnode_dirdeclarator {
//	_ASTNODE
//
//	enum declarator_type {DT_REGULAR, DT_ARRAY, DT_FN} declarator_type;
//	int is_abstract;
//
//	// this can be either an NT_IDENT or NT_DIRDECLARATOR -- check type
//	// before using it
//	union astnode *ident;
//
//	// if function
//	union astnode *paramtypelist;
//
//	// if array (assume size is an integer literal for now)
//	union astnode *size;
//
//	// when in function declarator; these will be moved to the pointer
//	union astnode *typequallist;
//
//	// NOTE: we are not doing anything with the STATIC keyword
//};
//
//struct astnode_paramdecl {
//	_ASTNODE
//
//	union astnode *declspec, *declarator;
//};
//
//struct astnode_typename {
//	_ASTNODE
//
//	union astnode *specquallist, *absdeclarator;
//};
//
//struct astnode_variable {
//	_ASTNODE
//
//	union astnode *declspec, *declarator;
//};

// defined in asttypes.c
//extern union astnode ELLIPSIS_DECLARATOR;
//
//// insert astnode into symbol table
//void insert_into_symtab(union astnode *declarator, union astnode *declspec,
//	enum name_space ns);
//
//// merge two declaration specifiers with all the rules; returns the merged
//// declspec and frees the other
//union astnode *merge_declspec(union astnode *, union astnode *);
//
//// print structs/unions after definition
//void print_structunion_def(union astnode *node);

#endif // ATTNODEH