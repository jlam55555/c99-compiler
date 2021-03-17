#include <stdio.h>
#include "parser.h"
#include "astnodegeneric.h"
#include "astnode.h"
#include "declarator.h"

union astnode *declarator_new(char *ident)
{
	union astnode *declarator_node;

	ALLOC_TYPE(declarator_node, NT_DECLARATOR);
	declarator_node->declarator.ident = strdup(ident);

	return declarator_node;
}

union astnode *declarator_append(union astnode *declarator_node,
	union astnode *component_node)
{
	struct astnode_declarator *declarator = &declarator_node->declarator;
	struct astnode_declarator_component *component =
		&component_node->declarator_component;

	component->next = declarator->components;
	declarator->components = component_node;

	return declarator_node;
}

void declarator_reverse(union astnode *declarator_node)
{
	// linear in-place singly-linked linked-list reversal
	union astnode *a, *b, *c;

	a = declarator_node->declarator.components;
	if (!a) {
		// empty components list, nothing to do
		return;
	}

	b = LL_NEXT(a);
	if (!b) {
		// components list is singleton, nothing to do
		return;
	}

	c = LL_NEXT(b);
	if (!c) {
		// components list is two elements a->b, reverse to b->a
		LL_NEXT(b) = a;
		LL_NEXT(a) = NULL;
		declarator_node->declarator.components = b;
		return;
	}

	// iterate
	LL_NEXT(a) = NULL;
	while (c) {
		LL_NEXT(b) = a;
		a = b;
		b = c;
		c = LL_NEXT(c);
	}
	LL_NEXT(b) = a;
	declarator_node->declarator.components = b;
}

void declarator_print(union astnode *component_node, int depth)
{
	FILE *fp = stdout;

	// end of declarator chain
	if (!component_node) {
		return;
	}

	INDENT(depth);
	switch (component_node->generic.type) {
	case NT_DECLARATOR_ARRAY:
		fprintf(fp, "array (%d) of\n",
			component_node->declarator_array.length->num.num.int_val);
		break;
	case NT_DECLARATOR_POINTER:
		fprintf(fp, "pointer to\n");
		break;
	case NT_DECLARATOR_FUNCTION:
		fprintf(fp, "function returning\n");
		break;
	default:
		fprintf(fp, "unknown type %d in print_symbol\n",
			component_node->generic.type);
		return;
	}

	// recursively print
	declarator_print(component_node->declarator_component.next, depth+1);
}

union astnode *declarator_array_new(union astnode *length_node,
	union astnode *spec_node)
{
	union astnode *declarator_array_node;
	struct astnode_declarator_array *declarator_array;

	ALLOC_TYPE(declarator_array_node, NT_DECLARATOR_ARRAY);

	declarator_array = &declarator_array_node->declarator_array;
	declarator_array->length = length_node;
	declarator_array->spec = spec_node;

	// we are not implementing VLAs
	if (!length_node) {
		yyerror_fatal("VLAs not supported");
	}
	// we only support numeric constants
	// TODO: should also check for non-integer constants
	else if (length_node->generic.type != NT_NUMBER) {
		yyerror_fatal("array length must be a numeric constant");
	}

	return declarator_array_node;
}

