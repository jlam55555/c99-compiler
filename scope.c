#include "astnode.h"
#include "scope.h"
#include "parser.h"
#include "lexerutils/errorutils.h"
#include <stdio.h>
#include <malloc.h>

static struct scope *scope_stack = NULL;
static int scope_pos = -1, scope_stack_capacity = 0;

// scope begun, create namespaces/symbol tables for it
void scope_push(enum scope_type type) {
	int i;

	// grow scope_stack if necessary
	if (scope_stack_capacity <= scope_pos + 1) {
		// stack has never been initialized, give it an initial value
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
	for (i = 0; i < 3; i++) {
		symtab_init(&scope->ns[i]);
	}

	// set scope parameters
	scope->type = type;
	scope->filename = strdup(filename);
	scope->lineno = lineno;
}

// scope ended, destroy it
void scope_pop(void) {
	int i;

	if (!scope_pos) {
		yyerror_fatal("popping empty scope stack");
	}

	for (i = 0; i < 3; i++) {
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
		--current_scope);

	return search;
}

// do same as above but return scope object
struct scope *get_scope(char *ident, enum name_space ns) {
	int current_scope;
	union astnode *search = NULL;

	for (current_scope = scope_pos;
		current_scope >= 0 && !(search =
		symtab_lookup(&scope_stack[current_scope].ns[ns], ident));
		--current_scope);

	return current_scope >= 0 ? &scope_stack[current_scope] : NULL;
}

struct scope *get_current_scope()
{
	return &scope_stack[scope_pos];
}