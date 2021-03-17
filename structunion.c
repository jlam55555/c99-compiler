#include <malloc.h>
#include "astnode.h"
#include "asttypes.h"
#include "structunion.h"
#include "symtab.h"
#include "parser.h"
#include "scope.h"
#include "string.h"
#include "lexerutils/errorutils.h"

// stack of union astnodes currently being declared
static union astnode **su_decl_stack = NULL;
int su_decl_stack_pos = -1, su_decl_stack_capacity = 0;

union astnode *structunion_new(enum structunion_type type)
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
	su->spectype = ST_TAG;
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
	union astnode *node, *symbol;
	struct astnode_typespec_structunion *su;

	// if stack empty, error (shouldn't happen, indicates error w/ compiler)
	if (su_decl_stack_pos < 0) {
		yyerror_fatal("empty struct/union declaration stack");
	}

	// check if ident exists in the current tag namespace
	node = scope_lookup(ident, NS_TAG);

	// already declared in symbol table (in the current scope)
	if (node && (get_scope(ident, NS_TAG) == get_current_scope())) {
		node = node->symbol.value;
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
		su->is_being_defined = begin_def;
	}
	
	// not already declared in symbol table
	else {
		node = su_decl_stack[su_decl_stack_pos];
		su = &node->ts_structunion;

		// set name, is_being_defined
		su->ident = strdup(ident);
		su->is_being_defined = begin_def;

		// insert into symtab
		ALLOC(symbol);
		symbol->symbol.type = NT_SYMBOL;
		symbol->symbol.ident = su->ident;
		symbol->symbol.ns = NS_TAG;
		symbol->symbol.value = node;

		scope_insert(ident, NS_TAG, symbol);
	}
}

void structunion_install_member(union astnode *declarator,
	union astnode *specquallist)
{
	union astnode *node, *search, *iter, *var, *symbol;
	struct astnode_typespec_structunion *su;
	struct astnode_decl *decl;
	struct astnode_declspec *declspec;
	char *ident;

	// if stack empty, error (shouldn't happen, indicates error w/ compiler)
	if (su_decl_stack_pos < 0) {
		yyerror_fatal("empty struct/union declaration stack");
	}

	node = su_decl_stack[su_decl_stack_pos];
	su = &node->ts_structunion;
	decl = &declarator->decl;
	declspec = &specquallist->declspec;

	// if declarator not a pointer, make sure typespec is not incomplete
//	if (!declr->pointer) {
//		if (declspec->ts->generic.type == NT_TS_STRUCT_UNION
//			&& !declspec->ts->ts_structunion.is_complete) {
//			yyerror_fatal("field has incomplete type");
//		}
//	}
//
//	// if declarator has a pointer, then make it point to the correct type
//	// (same as for regular variables)
//	else {
//		iter = declr->pointer;
//		while (iter->ptr.to) {
//			iter = iter->ptr.to;
//		}
//		iter->ptr.to = specquallist;
//	}
//
//	// get identifier from declarator
//	iter = declr->dirdeclarator->dirdeclarator.ident;
//	while (iter->generic.type != NT_IDENT) {
//		iter = iter->dirdeclarator.ident;
//	}
//	ident = strdup(iter->ident.ident);

	// check if field exists in member symtab
	search = symtab_lookup(&su->members_ht, ident);
	if (search) {
		yyerror_fatal("duplicate member");
	}

	// insert into symtab and list
//	ALLOC(var);
//	var->variable.type = NT_VARIABLE;
//	var->variable.declspec = specquallist;
//	var->variable.declarator = declarator;

	ALLOC(symbol);
	symbol->symbol.type = NT_SYMBOL;
	symbol->symbol.ident = ident;
	symbol->symbol.ns = NT_IDENT;
	symbol->symbol.value = var;
	symtab_insert(&su->members_ht, ident, symbol);

	// install (symbol) into linked list
	if (!su->members) {
		su->members = symbol;
	} else {
		LL_APPEND(su->members, symbol);
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
	if (su->is_being_defined) {
		// push dummy struct/union scope (for printing purposes)
		scope_push(ST_STRUCTUNION);
		dummy_scope = get_current_scope();
		dummy_scope->filename = su->def_filename;
		dummy_scope->lineno = su->def_lineno;

		print_structunion_def(node);

		// pop dummy struct/union scope
		scope_pop();
	}

	// not being defined anymore
	su->is_being_defined = 0;

	// if was already complete, leave it; otherwise set it
	su->is_complete |= is_complete;

	// pop from stack
	--su_decl_stack_pos;

	return node;
}