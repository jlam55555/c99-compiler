#ifndef NUMUTILSH
#define NUMUTILSH

#include <parser/astnode.h>
#include <parser/declspec.h>

/**
 * constructs an integral constant literal
 * 
 * see struct astnode_typespec_scalar (declspec.h) for more information on the
 * parameters
 * 
 * @param radix		integer radix (2, 8, 16, 10)
 * @param type		basetype
 * @param lls		long/long long/short modifiers
 * @param sign		signed/unsigned modifiers
 * @return		constructed astnode representing the number
 */
union astnode *make_int(int radix, enum scalar_basetype type,
	enum scalar_lls lls, enum scalar_sign sign);

/**
 * constructs a floating point constant literal
 * 
 * @param type		basetype
 * @param lls		set to LLS_LONG for long double type
 * @return		constructed astnode representing the number
 */
union astnode *make_fp(enum scalar_basetype type, enum scalar_lls lls);

/**
 * constructs an integer constant 1
 * 
 * We could probably create a single instance of an union astnode number literal
 * constant containing 1, but this creates a new instance when needed (in case
 * the representation ever has to be modified, just to be safe.)
 * 
 * @return		astnode number literal constant representation of 1
 */
union astnode *make_one(void);

// TODO: phase this out for astnode numeric type
// enum sign {
//     UNSIGNED_T,
//     SIGNED_T,
// };

// // TODO: phase this out for astnode numeric type
// enum type {
//     INT_T,
//     LONG_T,
//     LONGLONG_T,
//     DOUBLE_T,
//     LONGDOUBLE_T,
//     FLOAT_T,
// };

// // holds a numeric constant
// struct number {
//     // TODO: phase these out for astnode numeric type
// 	int type;
// 	int sign;

//     // numeric typespec (should be a struct NT_TS_SCALAR)
//     // TODO: working on this
//     // union astnode *ts;

//     // holding the byte values
// 	long long int int_val;
// 	long double real_val;
// };


// TODO: remove
// helper functions to make struct numbers for lexer
// struct number make_int(int radix, enum sign sign, enum type type);
// struct number make_fp(enum type type);

// helper function to print a number in the desired format;
// returns a dynamically-allocated string that should be freed after use
// char *print_number(struct number num);

#endif