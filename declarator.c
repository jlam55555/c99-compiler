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

	component->of = declarator->components;
	declarator->components = component_node;

	return declarator_node;
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
		fprintf(fp, "array of\n");
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
	declarator_print(component_node->declarator_component.of, depth+1);
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

	return declarator_array_node;
}
