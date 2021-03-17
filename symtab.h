/**
 * 	Data types and utilities for managing a symbol table. The symbol table
 * 	is a hashtable (mapping char * -> union astnode *) with automatic
 * 	rehashing.
 *
 * 	This is used in the regular symbol tables in scope.h, as well as the
 * 	"mini" symbol tables in structunion.h.
 *
 * 	TODO: these symbol tables never get freed (memory allocation is scary)
 */

#ifndef SYMTABH
#define SYMTABH

#include "common.h"
#include "parser.h"

struct symtab {
	union astnode **bs;
	int size, capacity;
};

// an entry in the symbol table
struct astnode_symbol {
	_ASTNODE

	char *ident;
	union astnode *value;
};

void *symtab_init(struct symtab *st);
void symtab_destroy(struct symtab *st);
int symtab_insert(struct symtab *st, char *ident, union astnode *node);
union astnode *symtab_lookup(struct symtab *st, char *ident);

#endif // SYMTABH