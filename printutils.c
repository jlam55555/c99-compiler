#include <stdio.h>
#include "astnode.h"
#include "structunion.h"
#include "printutils.h"

void print_structunion_def(union astnode *node)
{
	FILE *fp = stdout;
	union astnode *iter;
	struct astnode_typespec_structunion *su;

	su = &node->ts_structunion;

	fprintf(fp, "struct %s definition at %s:%d{\n",
		su->ident, su->def_filename, su->def_lineno);

	// loop through fields
	iter = su->members;
	while (iter) {
		//print_symbol(iter, 0);
		iter = iter->generic.next;
	}

	fprintf(fp, "}\n");
}
