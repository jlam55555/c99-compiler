/**
 * 	Helpers to declare new struct/union types, potentially recursively.
 * 	Stores the current struct/union declarations on a global stack, similar
 * 	to scope.c. Uses symtab.c hashtable implementation.
 */

#ifndef STRUCTUNIONH
#define STRUCTUNIONH

#include "astnode.h"

enum structunion_type { SU_STRUCT, SU_UNION };
enum structunion_complete { SU_COMPLETE, SU_INCOMPLETE };

// create new struct/union type and push onto stack
union astnode *structunion_new(enum structunion_type type);

// this will set the name of the current struct/union; returns:
// - itself if not already defined in the current scope's tag namespace
// - the existing definition if already defined in current scope's tag namespace
//   (and will free the current one)
void structunion_set_name(char *ident);

// add new member to the current struct being defined
void structunion_install_member(union astnode *declarator,
	union astnode *specquallist);

// pop current struct/union type from stack and set complete if is_complete,
// returns it
union astnode *structunion_done(enum structunion_complete is_complete);

#endif