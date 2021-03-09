/**
 *	This file includes utilities for typing that are too long to fit
 *	comfortably in a macro.
 */

#include "astnode.h"
#include "asttypes.h"
#include "symtab.h"
#include "parser.h"
#include <stdio.h>

// declare constant ellipsis for use; this is declared in asttypes.c
union astnode ELLIPSIS_DECLARATOR;

// for LL_APPEND to work
union astnode *ll_append_iter;

void print_symtab(union astnode *node, int depth)
{
	FILE *outfile = stdout;
	switch(node->generic.type)
	{
		case NT_TS_SCALAR:
			if(node->ts_scalar.modifiers.sign==SIGN_UNSIGNED)
				fprintf(outfile, "unsigned");
			switch(node->ts_scalar.modifiers.lls)
			{
				case LLS_UNSPEC:
					break;
				case LLS_SHORT:
					fprintf(outfile, "short ");
					break;
				case LLS_LONG:
					fprintf(outfile, "long ");
					break;
				case LLS_LONG_LONG:
					fprintf(outfile, "long long ");
					break;
			}
			switch(node->ts_scalar.basetype)
			{
				case BT_UNSPEC:
					fprintf(outfile, "\n");
					break;
				case BT_INT:
					fprintf(outfile, "int\n");
					break;
				case BT_FLOAT:
					fprintf(outfile, "float\n");
					break;
				case BT_DOUBLE:
					fprintf(outfile, "double\n");
					break;
				case BT_CHAR:
					fprintf(outfile, "char\n");
				case BT_BOOL:
					fprintf(outfile, "bool\n");

			}
			break;
		
		case NT_POINTER:
			if(node->ptr.typequallist)
				fprintf(outfile, "%s pointer to\n", print_typequallist(node->ptr.typequallist->tq.qual));
			else
				fprintf(outfile, "pointer to\n");
			print_symtab(node->ptr.to, depth+2);
			break;

		case NT_SYMBOL:
			/*fprintf(outfile, "%s is defined in %s:%d [in %s scope starting at %s:%d] as a ",
			 	node->symbol.ident);*/
			break;

		

	}


}

char *print_typequallist(unsigned char tq){
	return 0;

}

// insert value into symbol table
void insert_into_symtab(union astnode *declarator, union astnode *declspec,
	enum name_space ns)
{
	char *ident;
	union astnode *symbol, *iter;

	// get declarator name/identifier
	iter = declarator->declarator.dirdeclarator;
	while (iter->generic.type != NT_IDENT) {
		iter = iter->dirdeclarator.ident;
	}
	ident = iter->ident.ident;

	// if declarator has a pointer, then make it point to the correct type
	if (declarator->declarator.pointer) {
		iter = declarator->declarator.pointer;
		while (iter->ptr.to) {
			iter = iter->ptr.to;
		}
		iter->ptr.to = declspec;
	}

	// create symbol and insert
	ALLOC(symbol);
	symbol->symbol.type = NT_SYMBOL;
	symbol->symbol.declspec = declspec;
	symbol->symbol.declarator = declarator;
	symbol->symbol.ident = ident;

	#if DEBUG
	printf("Declaring symbol %s with type %d\n", ident,
		declspec->declspec.ts->ts_scalar.basetype);
	#endif

	scope_insert(ident, ns, symbol);
}

// combine declaration specifiers in the intended manner
union astnode *merge_declspec(union astnode *spec1, union astnode *spec2) {
	struct astnode_declspec ds1 = spec1->declspec,
		ds2 = spec2->declspec;
	struct astnode_typespec_scalar ats1, ats2;
	union astnode *iter;

	/* merge typespecs: combine all of them */
	if (ds1.ts) {
		if (ds2.ts) {
			ats1 = ds1.ts->ts_scalar, ats2 = ds2.ts->ts_scalar;

			// cannot have multiple base types
			if (ats1.basetype && ats2.basetype && ats1.basetype != ats2.basetype) {
				// TODO: fix these warnings
				fprintf(stderr, "cannot have multiple types\n");
				// _exit(0);
			}

			// TODO:
			// cannot have more than 2 longs

			// cannot have unsigned and signed

			// can only have long with double/int

			// can only have long long with int

			// cannot have duplicates except long
			
			// can bools be signed?
		}
	} else {
		ds1.ts = ds2.ts;
	}

	// merge typequals: combine all of them
	if (ds1.tq) {
		if (ds2.tq) {
			ds1.tq->tq.qual |= ds2.tq->tq.qual;
		}
	} else {
		ds1.tq = ds2.tq;
	}

	// merge scspecs: only allow one, error if multiple distinct scs
	if (ds1.sc) {

		// multiple storage classes
		if (ds2.sc && ds2.sc->sc.type != ds1.sc->sc.type) {
			// TODO: proper warning message
			fprintf(stderr, "Error: multiple storage class specifiers\n");
			// _exit(0);
		}

		// duplicate storage class
		if (ds2.sc) {
			fprintf(stderr, "Error: Duplicate storage class specifier");
			// _exit(0);
		}
	} else {
		ds1.sc = ds2.sc;
	}

	// cleanup
	free(spec2);
	return spec1;
}