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

#include <common.h>
#include <lexer/errorutils.h>

// linked list of all the global variables
extern union astnode *global_vars;

// need a second linked-list pointer *of since the generic *next may be used
// for linking declarators together (e.g., in parameter type list)
#define _ASTNODE_DECLARATOR_COMPONENT\
	_ASTNODE\
	union astnode *of, *spec;\

// special linked list macros since this uses a different field name
// (regular LL_* macros use generic.next)
#define LL_APPEND_OF(ll, node) _LL_APPEND(ll, node, decl_component.of)
#define LL_NEXT_OF(ll) _LL_NEXT(ll, decl_component.of)
#define LL_FOR_OF(ll, iter) _LL_FOR(ll, iter, decl_component.of)

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

	// scope which this symbol is inserted into
	struct scope *scope;

	// whether this variable comes from a prototype scope (for local
	// variables only); because prototype scope become incorporated into
	// local scope, have to distinguish it manually here
	int is_proto;

	// linked list of components -- for convenience, insert in reversed
	// order, and then reverse when complete using when calling
	// decl_finalize()
	union astnode *components;

	// declspec (also redundantly in the component chain, because it
	// includes the typespec -- in hindsight it would've been smarter to
	// separate storage class from the typespec)
	union astnode *declspec;

	// function body (for defined functions only)
	union astnode *fn_body;

	// for debugging purposes
	char *filename;
	int lineno;

	// indicates whether it is an implicit declaration
	int is_implicit;

	// need a list of global symbols for target code generation
	union astnode *global_next;
	
	// for local variables: need offset for target code generation
	int offset;
};

/**
 * create a new astnode_decl object (for both regular and abstract declarators)
 * 
 * @param ident		identifier of declarator; set to NULL if abstract
 * @return		new astnode_decl object
 */
union astnode *decl_new(char *ident);

/**
 * append a (multiple) declarator component(s) to a declarator; inserts in
 * reverse order (will be reversed when calling decl_finalize()
 *
 * @param decl 		astnode_decl to add component to
 * @param components 	ll of astnode_decl_*
 * @return 		decl (redundant of first parameter for convenience)
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

/**
 * install variable/function into the regular namespace in the current scope
 *
 * @param decl 		declarator
 * @param declspec 	declspec
 */
void decl_install(union astnode *decl, union astnode *declspec);

/**
 * check that the declarator of a function (declaration or definition) is valid,
 * and perform appropriate transformations. Will not have any effect if decl
 * is not a function declaration/definition
 * 
 * will perform the following checks:
 * - doesn't return an array type
 * 
 * will perform the following transformations:
 * - arrays in fn parameter list will be converted to pointers
 * 
 * @param decl		declarator of a function (declaration or definition)
 */
void decl_check_fndecl(union astnode *decl);

/**
 * check that the declarator of a function definition is valid:
 * - no abstract parameters
 * - declarator is indeed a function declarator
 * 
 * Will call decl_check_fndecl to also perform those checks.
 * 
 * Will yyerror_fail on error.
 * 
 * @param decl		declarator of a function definition
 */
void decl_check_fndef(union astnode *decl);

#endif // DECLARATORH