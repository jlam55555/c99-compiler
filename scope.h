/**
 * 	Scope management functions. Allows creating/popping slopes. Note that
 * 	the global (file) scope should be created by calling
 * 	`scope_push(ST_FILE)` before calling yyparse(). The scopes are stored
 * 	recursively on a stack, and each scope contains the three namespace
 * 	types. Does not include "mini symbol tables" used for structs/unions --
 * 	these are handled separately in structunion.c. Uses symtab.c hashtable
 * 	implementation.
 */

#ifndef SCOPEH
#define SCOPEH
#include "asttypes.h"
#include "symtab.h"

// in asttypes.h:
// enum name_space { NS_TAG, NS_LABEL, NS_IDENT }; 
enum scope_type { ST_FILE, ST_FUNC, ST_BLOCK, ST_PROTO };

struct scope {
	enum scope_type type;
	struct symtab ns[3];
};

void scope_push(enum scope_type type);
void scope_pop(void);
void scope_insert(char *ident, enum name_space ns, union astnode *node);
union astnode *scope_lookup(char *ident, enum name_space ns);
struct scope *get_scope(char *ident, enum name_space ns);

struct scope *get_current_scope();

#endif	// SCOPEH