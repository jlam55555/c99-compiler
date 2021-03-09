#include "astnode.h"
#include "symtab.h"
#include <stdio.h>
#include <malloc.h>

static struct scope *scope_stack;
static int scope_pos = -1, scope_stack_capacity = 0;

// good hash primes: https://planetmath.org/goodhashtableprimes
static int ghp[] = {53, 97, 193, 389, 769, 1533, 3079, 6151, 12289, 24593};
static struct symtab *symtab_init(struct symtab *st) {
	st->size = 0;
	st->capacity = ghp[0];
	st->bs = (union astnode **) calloc(st->capacity, sizeof(union astnode));
}

static void symtab_destroy(struct symtab *st) {
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

static int symtab_insert(struct symtab *st, char *ident, union astnode *node) {
	int i;

	// resize hashtable if necessary
	if (st->size >= st->capacity / 2) {
		// find next "good hash prime"
		for (i = 0; i < 10 && ghp[i] != st->capacity; i++);
		if (i == 9) {
			fprintf(stderr, "error: maximum symbol hashtable"
				" capacity exceeded");
			return -1;
		}

		// TODO: have to rehash each value; this won't work
		st->capacity = ghp[i+1];
		st->bs = reallocarray(st->bs, st->capacity,
			sizeof(union astnode *));
	}

	// hash and linear probe; if name already exists then error
	for (i = symtab_hash(ident) % st->capacity;
		st->bs[i] && strcmp(ident, st->bs[i]->symbol.ident);
		i = (i+1) % st->capacity);

	// error: identifier already exists in this symbol table
	if (st->bs[i] && !strcmp(ident, st->bs[i]->symbol.ident)) {
		fprintf(stderr, "error: symbol %s already exists in symtab\n",
			ident);
		return -1;
	}

	st->bs[i] = node;
	++st->size;
	return 0;
}

static union astnode *symtab_lookup(struct symtab *st, char *ident) {
	int i;

	for (i = symtab_hash(ident) % st->capacity;
		st->bs[i] && strcmp(ident, st->bs[i]->symbol.ident);
		i = (i+1) % st->capacity);

	if (st->bs[i] && 1 /* TODO: check if identifier matches name */) {
		return st->bs[i];
	}
	return NULL;
}

// scope begun, create namespaces/symbol tables for it
void scope_push(enum scope_type type) {
	int i;

	// grow scope_stack if necessary
	if (scope_stack_capacity <= scope_pos + 1) {
		// scope stack has never been initialized, give it a good
		// initial value
		if (!scope_stack_capacity) {
			scope_stack_capacity = 16;		
		} else {
			scope_stack_capacity *= 2;
		}
		scope_stack = reallocarray(scope_stack, scope_stack_capacity,
			sizeof(struct scope));
	}

	// create scope
	struct scope *scope = &scope_stack[++scope_pos];
	for (i = 0; i < 4; i++) {
		symtab_init(&scope->ns[i]);
	}
	scope->type = type;
}

// scope ended, destroy it
void scope_pop(void) {
	int i;

	if (!scope_pos) {
		fprintf(stderr, "error: popping empty scope stack\n");
		return;
	}

	for (i = 0; i < 4; i++) {
		symtab_destroy(&scope_stack[scope_pos].ns[i]);
	}
	--scope_pos;
}

// inserts symbol at enclosing scope (or at function scope for labels)
void scope_insert(char *ident, enum name_space ns, union astnode *node) {
	symtab_insert(&scope_stack[scope_pos].ns[ns], ident, node);
}

// traverses up the stack to lookup a symbol
union astnode *scope_lookup(char *ident, enum name_space ns) {
	int current_scope;
	union astnode *search = NULL;

	for (current_scope = scope_pos;
		current_scope >= 0 && !(search =
		symtab_lookup(&scope_stack[current_scope].ns[ns], ident));
		--current_scope) {}

	return search;
}