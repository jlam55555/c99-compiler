#include <stdio.h>
#include "parser.h"
#include "astnodegeneric.h"
#include "astnode.h"
#include "decl.h"

union astnode *decl_new(char *ident)
{
	union astnode *decl;

	ALLOC_TYPE(decl, NT_DECLARATION);
	decl->decl.ident = strdup(ident);

	return decl;
}

union astnode *decl_append(union astnode *decl,
	union astnode *components)
{
	LL_APPEND(components, decl->decl.components);
	decl->decl.components = components;

	return decl;
}

static void decl_reverse(union astnode *decl)
{
	// linear in-place singly-linked linked-list reversal
	union astnode *a, *b, *c;

	// components list is < 2 elements, nothing to do
	if (!(a = decl->decl.components)
		|| !(b = LL_NEXT(a))) {
		return;
	}

	// initial setup
	c = LL_NEXT(b);
	LL_NEXT(a) = NULL;

	// iterate
	while (c) {
		LL_NEXT(b) = a;
		a = b;
		b = c;
		c = LL_NEXT(c);
	}
	LL_NEXT(b) = a;
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

void decl_print(union astnode *component, int depth)
{
	FILE *fp = stdout;

	// end of declarator chain
	if (!component) {
		return;
	}

	INDENT(depth);
	switch (component->generic.type) {
	case NT_DECLARATOR_ARRAY:
		fprintf(fp, "array (%d) of\n",
			component->decl_array.length->num.num.int_val);
		break;
	case NT_DECLARATOR_POINTER:
		fprintf(fp, "pointer to\n");
		break;
	case NT_DECLARATOR_FUNCTION:
		fprintf(fp, "function returning\n");
		break;
	default:
		fprintf(fp, "unknown type %d in print_symbol\n",
			component->generic.type);
		return;
	}

	// recursively print
	decl_print(component->decl_component.next, depth+1);
}

union astnode *decl_array_new(union astnode *length,
	union astnode *spec)
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
	else if (length->generic.type != NT_NUMBER) {
		yyerror_fatal("array length must be a numeric constant");
	}

	return decl_array;
}

union astnode *decl_pointer_new(union astnode *spec)
{
	union astnode *decl_pointer;

	ALLOC_TYPE(decl_pointer, NT_DECLARATOR_POINTER);
	decl_pointer->decl_pointer.spec = spec;
	return decl_pointer;
}

union astnode *decl_function_new(union astnode *paramdecls)
{
	union astnode *decl_function;

	ALLOC_TYPE(decl_function, NT_DECLARATOR_FUNCTION);
	decl_function->decl_function.paramlist = paramdecls;
	return decl_function;
}
