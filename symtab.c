#include "astnode.h"

enum namespace { NS_TAG, NS_LABEL, NS_BLOCK, NS_IDENT }; 
enum scopetype { ST_FILE, ST_FUNC, ST_BLOCK, ST_PROTO };

struct scope {
	struct symtab ns[4];
};

static struct scope *scope_stack;
static int scope_pos = -1, scope_stack_capacity = 0;

// scope begun, create namespaces/symbol tables for it
void scope_push() {
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
}

// scope ended, destroy it
void scope_pop() {
	int i;

	if (!scope_pos) {
		// TODO: panic: stack empty
		fprintf("error: popping empty scope stack\n");
		return;
	}

	for (i = 0; i < 4; i++) {
		symtab_destroy(&scope_stack[scope_pos].ns[i]);
	}
	--scope_pos;
}

struct symtab {
	union astnode **bs;
	int size, capacity;
};

// good hash primes: https://planetmath.org/goodhashtableprimes
static int ghp[] = {53, 97, 193, 389, 769, 1533, 3079, 6151, 12289, 24593};
static struct symtab *symtab_init(struct symtab *st) {
	st->size = 0;
	st->capacity = ghp[0];
	st->bs = (union astnode *) calloc(st->capacity, sizeof(union astnode));
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

void symtab_insert() {
	// resize hashtable if necessary
	// TODO

	// hash and linear probe
	// TODO
}

void symtab_lookup() {
	// hash and linear probe
	// TODO
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