#include <stdio.h>
#include "parser.h"
#include "astnodegeneric.h"
#include "astnode.h"
#include "declarator.h"

void declarator_set_ident(union astnode *node, char *ident)
{
	node->declarator.ident = strdup(ident);
}

void declarator_print(union astnode *node, int depth)
{
	FILE *fp = stdout;

	// end of declarator chain
	if (!node) {
		return;
	}

	INDENT(depth);
	switch (node->generic.type) {
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
			node->generic.type);
		return;
	}

	// recursively print
	declarator_print(node->declarator_component.of, depth+1);
}
