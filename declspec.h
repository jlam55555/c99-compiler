/**
 * 	Types and utilities for dealing with declaration specifiers: (scalar)
 * 	type specifiers, storage class specifiers, and type qualifiers.
 */

#ifndef DECLSPECH
#define DECLSPECH

#include "common.h"

// typespec: scalar types
#define TS_SCALAR_SIGNED	0x4
#define TS_SCALAR_LL		0x2
#define TS_SCALAR_L		0x1
struct astnode_typespec_scalar {
	_ASTNODE

	enum scalar_basetype {
		BT_UNSPEC, BT_CHAR, BT_VOID, BT_INT,
		BT_DOUBLE, BT_FLOAT, BT_BOOL
	} basetype;

	struct modifiers {
		enum {LLS_UNSPEC, LLS_LONG, LLS_LONG_LONG, LLS_SHORT} lls;
		enum {SIGN_UNSPEC, SIGN_SIGNED, SIGN_UNSIGNED} sign;
	} modifiers;
};

// type qualifiers
#define TQ_CONST	0x1
#define TQ_RESTRICT	0x2
#define TQ_VOLATILE	0x4
struct astnode_typequal {
	_ASTNODE

	unsigned char qual;
};

// storage class
struct astnode_storageclass {
	_ASTNODE

	enum sc_spec { SC_EXTERN, SC_STATIC, SC_AUTO, SC_REGISTER } scspec;
};

// composite declaration specifiers class
struct astnode_declspec {
	_ASTNODE

	union astnode *ts, *tq, *sc;
};

/**
 * merge two declaration specifers following the standard semantic rules;
 * returns the merged declspec and frees the other
 *
 * @param ds1	first declaration specifier
 * @param ds2	second declaration specifier
 * @return	merged declaration specifier
 */
union astnode *merge_declspec(union astnode *ds1, union astnode *ds2);

/**
 * filling in the missing fields of a declspec appropriate given the context
 * (e.g., default storage class is extern in global scope, auto otherwise)
 *
 * @param declspec	declspec to fill in defaults for
 */
void declspec_fill_defaults(union astnode *declspec);

#endif // DECLSPECH