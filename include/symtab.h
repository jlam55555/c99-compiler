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

#include "parser.h"

// an entry in the symbol table; basically a key-value pair; this is a plain
// C struct and not an astnode because it doesn't have to be
struct symbol {
	char *ident;
	union astnode *value;
};

// symbol table struct (a hashtable)
struct symtab {
	struct symbol **bs;
	int size, capacity;
};

/**
 * functions for symtab management
 *
 * @param st 	symbol table
 */
void *symtab_init(struct symtab *st);
void symtab_destroy(struct symtab *st);

/**
 * insert symbol into symtab
 *
 * @param st 	symbol table
 * @param ident symbol key
 * @param node 	astnode_symbol instance
 */
void symtab_insert(struct symtab *st, char *ident, union astnode *node);

/**
 * lookup a key in the symbol table
 *
 * @param st	symbol table
 * @param ident	key to look up
 * @return	pointer to symbol if found, otherwise NULL
 */
union astnode *symtab_lookup(struct symtab *st, char *ident);

#endif // SYMTABH