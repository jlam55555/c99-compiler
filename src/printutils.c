#include <stdio.h>
#include "astnode.h"
#include "structunion.h"
#include "printutils.h"

void print_typespec(union astnode *node, int depth)
{
	FILE *fp = stdout;

	if (!node) {
		fprintf(fp, "unspecified type\n");
		return;
	}

	switch (node->generic.type) {

	// scalar types
	case NT_TS_SCALAR:
		// signedness (for applicable types)
		if(node->ts_scalar.modifiers.sign==SIGN_UNSIGNED)
			fprintf(fp, "unsigned");

		// long/long long/short (for applicable types)
		switch(node->ts_scalar.modifiers.lls) {
		case LLS_SHORT:		fprintf(fp, "short "); break;
		case LLS_LONG:		fprintf(fp, "long "); break;
		case LLS_LONG_LONG:	fprintf(fp, "long long "); break;
		}

		// "base" scalar type
		switch(node->ts_scalar.basetype) {
		case BT_INT: 		fprintf(fp, "int"); break;
		case BT_FLOAT: 		fprintf(fp, "float"); break;
		case BT_DOUBLE: 	fprintf(fp, "double"); break;
		case BT_CHAR: 		fprintf(fp, "char"); break;
		case BT_BOOL: 		fprintf(fp, "bool"); break;
		}
		fprintf(fp, "\n");
		break;

	// struct types: only need to print tag and where it was defined
	case NT_TS_STRUCT_UNION:;
		struct astnode_typespec_structunion *su = &node->ts_structunion;
		fprintf(fp, "struct %s ", su->ident);
		if (su->is_complete) {
			fprintf(fp, "(defined at %s:%d)\n",
				su->def_filename, su->def_lineno);
		} else {
			fprintf(fp, "(incomplete)\n", su->ident);
		}
		break;
	}
}

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

void print_declarator(union astnode *component, int depth)
{
	FILE *fp = stdout;

	// end of declarator chain
	if (!component) {
		return;
	}

	INDENT(depth);
	switch (component->generic.type) {
		// base case
		case NT_DECL:
			print_declarator(component->decl.components, depth);
			break;

		// end of declarator chain, typespec reached
		case NT_TS_SCALAR:
		case NT_TS_STRUCT_UNION:
			print_typespec(component, depth+1);
			break;

		// declarator components
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
	print_declarator(component->decl_component.next, depth+1);
}
