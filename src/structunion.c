#include <malloc.h>
#include "astnode.h"
#include "structunion.h"
#include "symtab.h"
#include "parser.h"
#include "scope.h"
#include "string.h"
#include "printutils.h"
#include "lexerutils/errorutils.h"

// stack of union astnodes currently being declared
static union astnode **su_decl_stack = NULL;
int su_decl_stack_pos = -1, su_decl_stack_capacity = 0;

void structunion_new(enum structunion_type type)
{
	union astnode *node;
	struct astnode_typespec_structunion *su;

	// grow stack if necessary
	if (su_decl_stack_capacity <= su_decl_stack_pos + 1) {
		// if stack has never been initialized, give it an initial value
		su_decl_stack_capacity = su_decl_stack_capacity
			? su_decl_stack_capacity * 2 : 16;

		su_decl_stack = (union astnode **) reallocarray(su_decl_stack,
			su_decl_stack_capacity, sizeof(union astnode *));
	}

	// create new struct/union
	ALLOC(node);
	su = &node->ts_structunion;

	// fill in struct fields
	su->type = NT_TS_STRUCT_UNION;
	su->ident = NULL;
	su->members = NULL;
	su->su_type = type;
	symtab_init(&su->members_ht);
	su->is_complete = su->is_being_defined = 0;

	// debugging info
	su->def_filename = strdup(filename);
	su->def_lineno = lineno;

	// push onto stack
	su_decl_stack[++su_decl_stack_pos] = node;
}

void structunion_set_name(char *ident, int begin_def)
{
	union astnode *node;
	struct astnode_typespec_structunion *su;

	// if stack empty, error (shouldn't happen, indicates error w/ compiler)
	if (su_decl_stack_pos < 0) {
		yyerror_fatal("empty struct/union declaration stack");
	}

	// check if ident exists in the current tag namespace
	node = scope_lookup(ident, NS_TAG);

	// already declared in symbol table (in the current scope)
	if (node && (get_scope(ident, NS_TAG) == get_current_scope())) {
		su = &node->ts_structunion;

		// redefinition of completed type
		if (begin_def && su->is_complete) {
			yyerror_fatal("struct/union redefinition");
		}

		// redefinition of nested type
		if (begin_def && su->is_being_defined) {
			yyerror_fatal("nested struct/union redefinition");
		}

		// replace top of stack with found, set begin_def
		free(su_decl_stack[su_decl_stack_pos]);
		su_decl_stack[su_decl_stack_pos] = node;
	}
	
	// not already declared in symbol table
	else {
		node = su_decl_stack[su_decl_stack_pos];
		su = &node->ts_structunion;

		// set name, is_being_defined
		su->ident = strdup(ident);
		su->is_being_defined = begin_def;

		// insert into symtab
		scope_insert(ident, NS_TAG, node);
	}
}

void structunion_install_member(union astnode *decl,
	union astnode *declspec)
{
	union astnode *node, *search;
	struct astnode_typespec_structunion *su;
	char *ident;

	// if stack empty, error (shouldn't happen, indicates error w/ compiler)
	if (su_decl_stack_pos < 0) {
		yyerror_fatal("empty struct/union declaration stack");
	}

	node = su_decl_stack[su_decl_stack_pos];
	su = &node->ts_structunion;

	decl_finalize(decl, declspec);

	// check if field exists in member symtab
	ident = decl->decl.ident;
	search = symtab_lookup(&su->members_ht, ident);
	if (search) {
		yyerror_fatal("duplicate member");
	}

	// insert into symtab and list
	symtab_insert(&su->members_ht, ident, decl);

	// install (symbol) into linked list
	if (!su->members) {
		su->members = decl;
	} else {
		LL_APPEND(su->members, decl);
	}
}

union astnode *structunion_done(int is_complete)
{
	union astnode *node;
	struct astnode_typespec_structunion *su;
	struct scope *dummy_scope;

	// if stack empty, error (shouldn't happen, indicates error w/ compiler)
	if (su_decl_stack_pos < 0) {
		yyerror_fatal("empty struct/union declaration stack");
	}

	node = su_decl_stack[su_decl_stack_pos];
	su = &node->ts_structunion;

	// print out definition (only if just defined)
	if (su->is_being_defined && is_complete) {
		// push dummy struct/union scope (for printing purposes)
		scope_push(ST_STRUCTUNION);
		dummy_scope = get_current_scope();
		dummy_scope->filename = su->def_filename;
		dummy_scope->lineno = su->def_lineno;

		print_structunion_def(node);

		// pop dummy struct/union scope
		scope_pop();
	}

	if (is_complete) {
		su->is_being_defined = 0;
		su->is_complete = 1;
	}

	// pop from stack
	--su_decl_stack_pos;

	return node;
}