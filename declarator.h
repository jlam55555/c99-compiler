/**
 * Utility methods for parsing complex declarators. Creates a (backwards)
 * declarator "chain" of pointers, functions, and arrays; chain ultimately
 * ends in the declspec. Also for abstract declarators.
 *
 * The declaration struct is generic enough for regular declarations, function
 * parameters, and typenames.
 */

#ifndef DECLARATORH
#define DECLARATORH

#include "astnodegeneric.h"

struct astnode_declarator_pointer {
	_ASTNODE

	union astnode *of;

	// pointer spec
	union astnode *spec;
};

struct astnode_declarator_function {
	_ASTNODE

	union astnode *of;

	// can only include INLINE, which gets ignored anyway
	union astnode *spec;

	// paramlist
	union astnode *paramlist;
};

struct astnode_declarator_array {
	_ASTNODE

	union astnode *of;

	// for now can only hold NT_NUMBER type
	union astnode *length;
};

// for storing arbitrarily-complex regular or abstract declarators
struct astnode_declarator {
	_ASTNODE

	// ident is null for abstract declarators
	char *ident;

	// linked list of components -- for convenience, insert in reversed
	// order, and then reverse when complete
	union astnode *components;
};

// for storing regular variable/function declarations, param types, typenames
struct astnode_declaration {
	_ASTNODE

	struct astnode_declspec *declspec;
	struct astnode_declarator *declarator;
};

#endif // DECLARATORH