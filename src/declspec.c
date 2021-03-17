#include "declspec.h"
#include "astnode.h"

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
	spec1->declspec = ds1;
	return spec1;
}

void declspec_fill_defaults(union astnode *declspec)
{
	// TODO
}