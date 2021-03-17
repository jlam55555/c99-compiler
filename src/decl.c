#include <stdio.h>
#include "common.h"
#include "parser.h"
#include "astnode.h"
#include "decl.h"
#include "scope.h"
#include "printutils.h"

union astnode *decl_new(char *ident)
{
	union astnode *decl;

	ALLOC_TYPE(decl, NT_DECL);
	decl->decl.ident = strdup(ident);

	return decl;
}

union astnode *decl_append(union astnode *decl, union astnode *components)
{
	LL_APPEND_OF(components, decl->decl.components);
	decl->decl.components = components;

	return decl;
}

static void decl_reverse(union astnode *decl)
{
	// linear in-place singly-linked linked-list reversal
	union astnode *a, *b, *c;

	// components list is < 2 elements, nothing to do
	if (!(a = decl->decl.components) || !(b = LL_NEXT_OF(a))) {
		return;
	}

	// initial setup
	c = LL_NEXT_OF(b);
	LL_NEXT_OF(a) = NULL;

	// iterate
	while (c) {
		LL_NEXT_OF(b) = a;
		a = b;
		b = c;
		c = LL_NEXT_OF(c);
	}
	LL_NEXT_OF(b) = a;
	decl->decl.components = b;
}

void decl_finalize(union astnode *decl, union astnode *declspec)
{
	decl->decl.declspec = declspec;

	// redundantly append declspec to the end of the components ll
	// for convenience of printing
	decl_append(decl, declspec);

	// reverse declspec ll so that it is in the logical order
	decl_reverse(decl);
}

union astnode *decl_array_new(union astnode *length, union astnode *spec)
{
	union astnode *decl_array;
	struct astnode_decl_array *array;

	ALLOC_TYPE(decl_array, NT_DECLARATOR_ARRAY);

	array = &decl_array->decl_array;
	array->length = length;
	array->spec = spec;

	// we are not implementing VLAs
	if (!length) {
		yyerror_fatal("VLAs not supported");
	}
	// we only support numeric constants
	// TODO: should also check for non-integer constants
	else if (NT(length) != NT_NUMBER) {
		yyerror_fatal("array length must be a numeric constant");
	}

	return decl_array;
}

union astnode *decl_pointer_new(union astnode *spec)
{
	union astnode *decl_pointer;

	ALLOC_TYPE(decl_pointer, NT_DECLARATOR_POINTER);
	// spec is a declspec (because we use declspec_merge to merge them
	// but we only need to store a typequal list
	if (spec) {
		decl_pointer->decl_pointer.spec = spec->declspec.tq;
	}
	return decl_pointer;
}

union astnode *decl_function_new(union astnode *paramdecls)
{
	union astnode *decl_function;

	ALLOC_TYPE(decl_function, NT_DECLARATOR_FUNCTION);
	decl_function->decl_function.paramlist = paramdecls;
	return decl_function;
}

void decl_install(union astnode *decl, union astnode *declspec)
{
	FILE *fp = stdout;
	char *ident;

	// get ident from declarator
	ident = decl->decl.ident;

	// fill in missing fields of declspec
	declspec_fill_defaults(declspec);

	// combine declspec and decl to make full declaration
	decl_finalize(decl, declspec);

	// TODO: check that all of the declaration specifiers are valid
	//	e.g., function param list cannot have certain storage class spec
	//	https://en.cppreference.com/w/c/language/function_declaration
	//	e.g., void special parameter

#if DEBUG
	print_symbol(decl, 1);
#endif

	scope_insert(ident, NS_IDENT, decl);
}
