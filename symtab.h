#ifndef SYMTABH
#define SYMTABH

#include "stdio.h"
#include "parser.h"
#include "asttypes.h"

/* declared in asttypes.h
struct symtab {
	union astnode **bs;
	int size, capacity;
};*/

void *symtab_init(struct symtab *st);
void symtab_destroy(struct symtab *st);
int symtab_insert(struct symtab *st, char *ident, union astnode *node);
union astnode *symtab_lookup(struct symtab *st, char *ident);

#endif // SYMTABH