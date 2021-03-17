#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

// good hash primes: https://planetmath.org/goodhashtableprimes
static int ghp[] = {53, 97, 193, 389, 769, 1533, 3079, 6151, 12289, 24593};
void *symtab_init(struct symtab *st) {
	st->size = 0;
	st->capacity = ghp[0];
	st->bs = (struct symbol **) calloc(st->capacity,
		sizeof(struct symbol *));
}

void symtab_destroy(struct symtab *st) {
	int i;

	// free each astnode recursively
	for (i = 0; i < st->capacity; i++) {
		if (st->bs[i]) {
			// TODO: how to clean up node at st->bs[i] ??????????
			// free(st->bs[i].value);
		}
	}

	// free symtab
	free(st->bs);
}

// djb2: http://www.cse.yorku.ca/~oz/hash.html
static unsigned long symtab_hash(char *s) {
	unsigned long hash = 5381;
	int c;

	/* hash * 33 + c */
	while (c = *s++)
		hash = ((hash << 5) + hash) + c;

	return hash;
}

static void symtab_rehash(struct symtab *st) {
	int i;
	struct symbol **tmp, *iter, *end;

	// find next "good hash prime"
	for (i = 0; i < 10 && ghp[i] != st->capacity; i++);
	if (i == 9) {
		yyerror_fatal("maximum symbol hashtable capacity exceeded");
	}

	st->capacity = ghp[i+1];
	tmp = st->bs;
	st->bs = (struct symbol **) calloc(st->capacity,
		sizeof(struct symbol *));

	// rehash and reinsert all of the old elements
	for (iter = *tmp, end = *tmp+ghp[i]; iter < end; ++iter) {
		symtab_insert(st, iter->ident, iter->value);
		free(iter);
	}

	free(tmp);
}

void symtab_insert(struct symtab *st, char *ident, union astnode *node) {
	int i;
	struct symbol *symbol;

	// resize hashtable if necessary
	if (st->size >= st->capacity / 2) {
		symtab_rehash(st);
	}

	// hash and linear probe; if name already exists then error
	for (i = symtab_hash(ident) % st->capacity;
		st->bs[i] && strcmp(ident, st->bs[i]->ident);
		i = (i+1) % st->capacity);

	// identifier already exists in this symbol table
	if (st->bs[i] && !strcmp(ident, st->bs[i]->ident)) {
		yyerror_fatal("symbol already exists in symtab");
	}

	// allocate symbol
	symbol = malloc(sizeof(struct symbol));
	symbol->value = node;
	symbol->ident = ident;

	st->bs[i] = symbol;
	++st->size;
}

union astnode *symtab_lookup(struct symtab *st, char *ident) {
	int i;

	if (!st->capacity) {
		return NULL;
	}

	for (i = symtab_hash(ident) % st->capacity;
		st->bs[i] && strcmp(ident, st->bs[i]->ident);
		i = (i+1) % st->capacity);

	if (st->bs[i] && !(strcmp(ident, st->bs[i]->ident))) {
		return st->bs[i]->value;
	}
	return NULL;
}