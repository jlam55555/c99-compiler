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

	#if DEBUG
	printf("Declaring symbol %s with type %d\n", ident,
		declspec->declspec.ts->ts_scalar.basetype);
	#endif

	// scope_insert(ident, ns, symbol);
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