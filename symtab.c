#include "symtab.h"
#include "astnode.h"

// good hash primes: https://planetmath.org/goodhashtableprimes
static int ghp[] = {53, 97, 193, 389, 769, 1533, 3079, 6151, 12289, 24593};
void *symtab_init(struct symtab *st) {
	st->size = 0;
	st->capacity = ghp[0];
	st->bs = (union astnode **) calloc(st->capacity, sizeof(union astnode));
}

void symtab_destroy(struct symtab *st) {
	int i;

	// free each astnode recursively
	for (i = 0; i < st->capacity; i++) {
		if (st->bs[i]) {
			// TODO: how to clean up node at st->bs[i] ??????????
			free(st->bs[i]);
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
	union astnode **tmp, *iter, *end;

	// find next "good hash prime"
	for (i = 0; i < 10 && ghp[i] != st->capacity; i++);
	if (i == 9) {
		yyerror_fatal("maximum symbol hashtable capacity exceeded");
	}

	st->capacity = ghp[i+1];
	tmp = st->bs;
	st->bs = calloc(st->capacity, sizeof(union astnode *));

	// rehash and reinsert all of the old elements
	for (iter = *tmp, end = *tmp+ghp[i]; iter < end; ++iter) {
		symtab_insert(st, iter->symbol.ident, iter);
	}

	free(tmp);
}

int symtab_insert(struct symtab *st, char *ident, union astnode *node) {
	int i;

	// resize hashtable if necessary
	if (st->size >= st->capacity / 2) {
		symtab_rehash(st);
	}

	// hash and linear probe; if name already exists then error
	for (i = symtab_hash(ident) % st->capacity;
		st->bs[i] && strcmp(ident, st->bs[i]->symbol.ident);
		i = (i+1) % st->capacity);

	// error: identifier already exists in this symbol table
	// TODO: this should not be an error if current one is an extern and
	// matches previous declaration
	if (st->bs[i] && !strcmp(ident, st->bs[i]->symbol.ident)) {
		char buf[1024];
		snprintf(buf, sizeof(buf), "symbol %s already exists in symtab",
			ident);
		yyerror(buf);
		return -1;
	}

	st->bs[i] = node;
	++st->size;
	return 0;
}

union astnode *symtab_lookup(struct symtab *st, char *ident) {
	int i;

	for (i = symtab_hash(ident) % st->capacity;
		st->bs[i] && strcmp(ident, st->bs[i]->symbol.ident);
		i = (i+1) % st->capacity);

	if (st->bs[i] && 1 /* TODO: check if identifier matches name */) {
		return st->bs[i];
	}
	return NULL;
}