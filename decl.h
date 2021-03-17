/**
 * Utility methods for parsing complex declarators and declarations.
 * Creates a (backwards) declarator "chain" of pointers, functions, and arrays.
 * The chain ends when you call decl_finalize(), which adds the declspec
 * and reverses the order of the declarator components.
 *
 * This can be used for regular variable/function declarations, abstract
 * declarators and typenames, and parameter type lists.
 *
 * Note that these data structures and functions are intentionally ambiguous
 * (decl for both declaration and declarator) because the only difference
 * between them is the existence of the declaration specifier.
 */

#ifndef DECLARATORH
#define DECLARATORH

#include "common.h"

#define _ASTNODE_DECLARATOR_COMPONENT\
	_ASTNODE\
	union astnode *spec;\

struct astnode_decl_component {
	_ASTNODE_DECLARATOR_COMPONENT
};

struct astnode_decl_pointer {
	_ASTNODE_DECLARATOR_COMPONENT
};

struct astnode_decl_function {
	_ASTNODE_DECLARATOR_COMPONENT

	// paramlist
	union astnode *paramlist;
};

struct astnode_decl_array {
	_ASTNODE_DECLARATOR_COMPONENT

	// for now can only hold NT_NUMBER type
	union astnode *length;
};

struct astnode_decl {
	_ASTNODE

	// ident is null for abstract declarators
	char *ident;

	// linked list of components -- for convenience, insert in reversed
	// order, and then reverse when complete using when calling
	// decl_finalize()
	union astnode *components;

	// declspec (to be added
	union astnode *declspec;
};

// create a new astnode_decl object; ident=NULL for abstract
union astnode *decl_new(char *ident);

/**
 * append a (multiple) declarator component(s) to a declarator; inserts in
 * reverse order (will be reversed when calling decl_finalize()
 *
 * @param decl 		astnode_decl to add component to
 * @param components 	ll of astnode_decl_*
 * @return 		decl (for convenience)
 */
union astnode *decl_append(union astnode *decl, union astnode *components);

/**
 * adds declspec to declarator list and reverses list so that it is ready to be
 * inserted into the symbol table and/or printed; at this point declarator list
 * becomes a declaration/typename
 *
 * @param decl		decl
 * @param declspec	declspec to add to declaration
 */
void decl_finalize(union astnode *decl, union astnode *declspec);

/**
 * recursively print declarator components; initiate by calling on the
 * component list of a astnode_decl object
 *
 * @param component	ll of astnode_decl_*
 * @param depth		indenting depth
 */
void decl_print(union astnode *component, int depth);

/**
 * create new declarator components; set parameters to NULL if missing
 *
 * @param length 	length of array
 * @param spec 		declspecs
 * @param paramdecls	param declaration list
 * @return
 */
union astnode *decl_array_new(union astnode *length,
	union astnode *spec);
union astnode *decl_pointer_new(union astnode *spec);
union astnode *decl_function_new(union astnode *paramdecls);

#endif // DECLARATORH