/**
 * 	Helpers to declare new struct/union types, potentially recursively.
 * 	Stores the current struct/union declarations on a global stack, similar
 * 	to scope.c. Uses symtab.c hashtable implementation.
 * 
 * 	see also: struct astnode_ts_structunion in asttypes.h
 */

#ifndef STRUCTUNIONH
#define STRUCTUNIONH

#include "astnode.h"
#include "asttypes.h"

// create new struct/union type and push onto stack
union astnode *structunion_new(enum structunion_type type);

// this will set the name of the current struct/union
// 
// semantic notes:
// - if ident is already declared in the current namespace, then the current
//   struct/union is freed and replaced with the existing one
// - begin_def indicates whether this begins a definition or not.
//	- if begin_def is true and the struct/union is found in the current
//	  namespace and is complete, then redefinition error
//	- if begin_def is true and the tag name already is being declared in
//	  the current scope but incomplete, then nested redefinition error
// 	- if begin_def is false, then should be fine -- need to check later
//	  (in install_member) whether type is incomplete or not
// - this doesn't need to be called on unnamed struct/unions, since they are
//   not inserted into the symbol table (no chance for conflict/redefinition)
void structunion_set_name(char *ident, int begin_def);

// add new member to the current struct being defined
void structunion_install_member(union astnode *declarator,
	union astnode *specquallist);

// pop current struct/union type from stack and set complete if is_complete,
// returns it
union astnode *structunion_done(int is_complete);

#endif