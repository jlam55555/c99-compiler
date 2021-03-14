/**
 *	This file includes utilities for typing that are too long to fit
 *	comfortably in a macro.
 */

#include "astnode.h"
#include "asttypes.h"
#include "scope.h"
#include "parser.h"
#include "lexerutils/errorutils.h"
#include <stdio.h>

// declare constant ellipsis for use; this is declared in asttypes.c
union astnode ELLIPSIS_DECLARATOR;

// for LL_APPEND to work
union astnode *ll_append_iter;

char *print_scope(enum scope_type st);
char *print_typequallist(unsigned char tq);
char *print_sc(union astnode *scspec_node);
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
					if(node->ts_scalar.modifiers.lls == LLS_UNSPEC)
						fprintf(outfile, "int\n");
					else
						fprintf(outfile, "\n");
					break;
				case BT_FLOAT:
					fprintf(outfile, "float\n");
					break;
				case BT_DOUBLE:
					fprintf(outfile, "double\n");
					break;
				case BT_CHAR:
					fprintf(outfile, "char\n");
					break;
				case BT_BOOL:
					fprintf(outfile, "bool\n");
					break;
			}
			break;
		
		case NT_DIRDECLARATOR:
			switch(node->dirdeclarator.declarator_type)
			{
				case DT_ARRAY:
					fprintf(outfile, "array of %d elements of type\n", node->dirdeclarator.size);
					print_symtab(node->dirdeclarator.typequallist, depth+2);
					break;

				case DT_FN:
					fprintf(outfile, "function returning\n");
					break;


			}
			break;
		
		case NT_POINTER:
			if(node->ptr.typequallist)
			{
				fprintf(outfile, "%s ", print_typequallist(node->ptr.typequallist->tq.qual));
				
			}
			fprintf(outfile, "pointer to\n");
			print_symtab(node->ptr.to, depth+2);
			break;

		case NT_SYMBOL:;
			struct scope *curscope = get_current_scope();
			fprintf(outfile, "%s is defined in %s:%d [in %s scope starting at %s:%d] as a ",
			 	node->symbol.ident, filename, lineno, print_scope(curscope->type), "", 1);
			INDENT(depth);
			switch(node->symbol.value->generic.type) {
				case NT_VARIABLE:
					fprintf(outfile, "variable with stgclass %s of type:\n", print_sc(node->symbol.value->declspec.sc));
					if(node->symbol.value->variable.declarator->declarator.pointer)
						print_symtab(node->symbol.value->variable.declarator->declarator.pointer, depth);
					else
					{
					
					INDENT(depth+1);			
					fprintf(outfile, "%s", "");
					}
					break;
				case NT_TS_STRUCT_UNION:
					// TODO: jon
					break;
				// case NT_LABEL:
			}
			
			break;

	}
	

}

char *print_typequallist(unsigned char tq)
{
	return "";
}


//Function to help print storage class
char *print_sc(union astnode *scspec_node)
{
	enum sc_spec scspec;

	if (!scspec_node) {
		return "";
	}

	scspec = scspec_node->sc.scspec;

	switch(scspec)
	{
		case SC_EXTERN:
			return "extern";
			break; 
		case SC_STATIC:
			return "static";
			break; 
		case SC_AUTO:
			return "auto";
			break;
		case SC_REGISTER:
			return "register";
			break;
		default:
			return "";
	}
}

//function to help print scope
char *print_scope(enum scope_type st)
{
	switch(st)
	{
		case ST_FILE:
			return "global";
			break;

		
		//ST_FUNC, ST_BLOCK, ST_PROTO
	}

}

// insert variable into symbol table
void insert_into_symtab(union astnode *declarator, union astnode *declspec,
	enum name_space ns)
{
	char *ident;
	union astnode *symbol, *iter, *var;

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

	ALLOC(var);
	var->variable.type = NT_VARIABLE;
	var->variable.declspec = declspec;
	var->variable.declarator = declarator;

	// create symbol and insert
	ALLOC(symbol);
	symbol->symbol.type = NT_SYMBOL;
	symbol->symbol.ident = ident;
	symbol->symbol.ns = ns;
	symbol->symbol.value = var;

	// TODO: if declspec has unspecified parts, fill them in with their
	// context-specific defaults
	// e.g., if storage class is unspecified, it should be set to extern if
	// global scope, otherwise auto

	print_symtab(symbol, 0);
 
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
	struct astnode_typespec_scalar *ats1, *ats2;
	union astnode *iter;
	int lls1, lls2, lls_res;

	/* merge typespecs: combine all of them */
	if (ds1.ts && ds1.ts->generic.type == NT_TS_SCALAR) {
		if (ds2.ts && ds1.ts->generic.type == NT_TS_SCALAR) {
			ats1 = &ds1.ts->ts_scalar, ats2 = &ds2.ts->ts_scalar;

			// COMBINING BASE TYPE
			// cannot have multiple base types
			if (ats1->basetype && ats2->basetype) {
				if (ats1->basetype != ats2->basetype) {
					yyerror_fatal("multiple type specifications in declaration");
				} else {
					yyerror("duplicate type specification in declaration");
				}
			} else if (ats2->basetype) {
				ats1->basetype = ats2->basetype;
			}

			// COMBINING LONG/LONG LONG/SHORT MODIFIER
			switch (ats1->modifiers.lls) {
			case LLS_LONG: lls1 = 1; break;
			case LLS_LONG_LONG: lls1 = 2; break;
			case LLS_SHORT: lls1 = -1; break;
			default: lls1 = 0;
			}
			switch (ats2->modifiers.lls) {
			case LLS_LONG: lls2 = 1; break;
			case LLS_LONG_LONG: lls2 = 2; break;
			case LLS_SHORT: lls2 = -1; break;
			default: lls2 = 0;
			}
			
			lls_res = 0;
			// cannot have more than 2 longs
			if (lls1 >= 0 && lls2 >= 0) {
				if ((lls_res = lls1 + lls2) > 2) {
					yyerror("integer type too long for C; truncating to long long");
					lls_res = 2;
				}
			}
			// cannot have long and short
			else if (lls1 > 0 && lls2 < 0 || lls1 < 0 && lls2 > 0) {
				yyerror_fatal("both long and short in declaration specifiers");
			}

			// set long/long long/short modifier with appropriate
			// merged type
			switch (lls_res) {
			case -1: ats1->modifiers.lls = LLS_SHORT; break;
			case 0: ats1->modifiers.lls = LLS_UNSPEC; break;
			case 1: ats1->modifiers.lls = LLS_LONG; break;
			case 2: ats1->modifiers.lls = LLS_LONG_LONG; break;
			}

			// can only have long long with int
			if (lls_res == 2) {
				switch (ats1->basetype) {
				case BT_INT: case BT_UNSPEC: break;
				default: yyerror_fatal("only int can have long long specifier");
				}
			}
			// can only have long with double/int
			else if (lls_res == 1) {
				switch (ats1->basetype) {
				case BT_INT: case BT_UNSPEC: case BT_DOUBLE: break;
				default: yyerror_fatal("only int or double can have long specifier");
				}
			}

			// COMBINING SIGN MODIFIER
			// cannot have unsigned and signed
			if (ats1->modifiers.sign && ats2->modifiers.sign) {
				if (ats1->modifiers.sign != ats2->modifiers.sign) {
					yyerror_fatal("both unsigned and signed in declaration specifiers");
				} else {
					yyerror("duplicate declaration specifier");
				}
			} else if (ats2->modifiers.sign) {
				ats1->modifiers.sign = ats2->modifiers.sign;
			}

			// cannot have unsigned float/double/bool
			if (ats1->modifiers.sign) {
				switch (ats1->basetype) {
				case BT_FLOAT: case BT_DOUBLE: case BT_BOOL:
					yyerror_fatal("non-integral type cannot be signed/unsigned");
				}
			}
		} else if (ds2.ts) {
			// ds1 is scalar, ds2 is also specified => multiple typespecs
			yyerror_fatal("multiple type specifiers in declaration");
		}
	} else if (ds1.ts) {
		// ds2 is scalar, ds1 is also specified => multiple typespecs
		if (ds2.ts) {
			yyerror_fatal("multiple type specifiers in declaration");
		}
	}
	// ds1 has no typespec just choose ds2's typespec
	else {
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
			yyerror_fatal("multiple storage class specifiers");
		}

		// duplicate storage class
		if (ds2.sc) {
			yyerror("duplicate storage class");
		}
	} else {
		ds1.sc = ds2.sc;
	}

	// cleanup
	free(spec2);
	return spec1;
}