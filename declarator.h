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

#define _ASTNODE_DECLARATOR_COMPONENT\
	_ASTNODE\
	union astnode *spec;\

struct astnode_declarator_component {
	_ASTNODE_DECLARATOR_COMPONENT
};

struct astnode_declarator_pointer {
	_ASTNODE_DECLARATOR_COMPONENT
};

struct astnode_declarator_function {
	_ASTNODE_DECLARATOR_COMPONENT

	// paramlist
	union astnode *paramlist;
};

struct astnode_declarator_array {
	_ASTNODE_DECLARATOR_COMPONENT

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

// create a new declarator; ident=NULL for abstract
union astnode *declarator_new(char *ident);

// append a (multiple) declarator component(s) to a declarator;
// returns declarator
union astnode *declarator_append(union astnode *declarator,
	union astnode *components);

// reverse declarator component order since they are inserted in reverse
void declarator_reverse(union astnode *declarator);

// recursively print declarator components
void declarator_print(union astnode *component, int depth);

// create new declarator components; set parameters to NULL if missing
union astnode *declarator_array_new(union astnode *length,
	union astnode *spec);
union astnode *declarator_pointer_new(union astnode *spec);
union astnode *declarator_function_new(union astnode *paramdecls);

#endif // DECLARATORH