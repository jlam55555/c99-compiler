#ifndef NUMUTILSH
#define NUMUTILSH

/* Define Sign and Types */
enum sign {
    UNSIGNED_T,
    SIGNED_T,
};

enum type {
    INT_T,
    LONG_T,
    LONGLONG_T,
    DOUBLE_T,
    LONGDOUBLE_T,
    FLOAT_T,
};

struct number {
	int type;
	int sign;
	long long int int_val;
	long double real_val;
};

// helper functions to make struct numbers for lexer
struct number make_int(int radix, enum sign sign, enum type type);
struct number make_fp(enum type type);

// helper function to print a number in the desired format;
// returns a dynamically-allocated string that should be freed after use
char *print_number(struct number num);

#endif