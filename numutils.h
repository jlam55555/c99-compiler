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

void print_number(struct number num);

#endif