/**
 * 	Helpers to declare new struct/union types, potentially recursively.
 * 	Stores the current struct/union declarations on a global stack, similar
 * 	to scope.c. Uses symtab.c hashtable implementation.
 * 
 * 	see also: struct astnode_ts_structunion in asttypes.h
 */

#ifndef STRUCTUNIONH
#define STRUCTUNIONH

#include "declspec.h"
#include "symtab.h"

enum structunion_type { SU_STRUCT, SU_UNION };

struct astnode_typespec_structunion {
	_ASTNODE

	char *ident;
	enum structunion_type su_type;

	// linked-list and hashtable of members as symbols
	union astnode *members;
	struct symtab members_ht;

	// to prevent multiple (nested) redefinition: see structunion.h
	int is_complete, is_being_defined;

	// for debugging purposes: prints out where struct is defined
	char *def_filename;
	int def_lineno;
};

/**
 * create new struct/union type and push onto stack
 *
 * @param type	indicate struct or union
 */
void structunion_new(enum structunion_type type);

/**
 * set the name of the current struct/union
 *
 * semantic notes (this function is dangerously innoculous):
 * - if ident is already declared in the current namespace, then the current
 *   struct/union is freed and replaced with the existing one
 * - begin_def indicates whether this begins a definition or not.
 *	- if begin_def is true and the struct/union is found in the current
 *	  namespace and is complete, then redefinition error
 *	- if begin_def is true and the tag name already is being declared in
 *	  the current scope but incomplete, then nested redefinition error
 * 	- if begin_def is false, then should be fine -- need to check later
 *	  (in install_member) whether type is incomplete or not
 * - this doesn't need to be called on unnamed struct/unions, since they are
 *   not inserted into the symbol table (no chance for conflict/redefinition)
 *
 * @param ident		struct tag
 * @param begin_def	whether this begins a struct definition or not
 */
void structunion_set_name(char *ident, int begin_def);

/**
 * add new member to the current struct being defined
 *
 * @param decl		declarator of member
 * @param declspec	declaration speciiers of member
 */
void structunion_install_member(union astnode *decl, union astnode *declspec);

/**
 * pop current struct/union type from stack and set complete if is_complete,
 * returns it
 *
 * @param is_complete	will mark struct/union as complete if this is set
 * @return		current struct/union type
 */
union astnode *structunion_done(int is_complete);

#endif