#include <stdio.h>
#include "parser.h"
#include "astnodegeneric.h"
#include "astnode.h"
#include "declarator.h"

union astnode *declarator_new(char *ident)
{
	union astnode *declarator;

	ALLOC_TYPE(declarator, NT_DECLARATOR);
	declarator->declarator.ident = strdup(ident);

	return declarator;
}

union astnode *declarator_append(union astnode *declarator,
	union astnode *components)
{
	LL_APPEND(components, declarator->declarator.components);
	declarator->declarator.components = components;

	return declarator;
}

void declarator_reverse(union astnode *declarator)
{
	// linear in-place singly-linked linked-list reversal
	union astnode *a, *b, *c;

	// components list is < 2 elements, nothing to do
	if (!(a = declarator->declarator.components)
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
	declarator->declarator.components = b;
}

void declarator_print(union astnode *component, int depth)
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
			component->declarator_array.length->num.num.int_val);
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
	declarator_print(component->declarator_component.next, depth+1);
}

union astnode *declarator_array_new(union astnode *length,
	union astnode *spec)
{
	union astnode *declarator_array;
	struct astnode_declarator_array *array;

	ALLOC_TYPE(declarator_array, NT_DECLARATOR_ARRAY);

	array = &declarator_array->declarator_array;
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

	return declarator_array;
}

union astnode *declarator_pointer_new(union astnode *spec)
{
	union astnode *declarator_pointer;

	ALLOC_TYPE(declarator_pointer, NT_DECLARATOR_POINTER);
	declarator_pointer->declarator_pointer.spec = spec;
	return declarator_pointer;
}

union astnode *declarator_function_new(union astnode *paramdecls)
{
	union astnode *declarator_function;

	ALLOC_TYPE(declarator_function, NT_DECLARATOR_FUNCTION);
	declarator_function->declarator_function.paramlist = paramdecls;
	return declarator_function;
}
