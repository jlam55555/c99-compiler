#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <parser/astnode.h>
#include <lexer/numutils.h>

extern char *yytext;

// struct number make_int(int radix, enum sign sign, enum type type) {
union astnode *make_int(int radix, enum scalar_basetype type,
	enum scalar_lls lls, enum scalar_sign sign)
{
	union astnode *number, *ts;

	// alloc and set union astnode type representation
	ALLOC_TYPE(ts, NT_TS_SCALAR);
	ts->ts_scalar.basetype = type;
	ts->ts_scalar.modifiers = (struct modifiers) {
		.lls = lls,
		.sign = sign,
	};

	ALLOC_TYPE(number, NT_NUMBER);
	number->num.ts = ts;
	*((unsigned long long*)number->num.buf) = strtoull(yytext, NULL, radix);

	return number;

	// return (struct number) {
	// 	.int_val = strtoull(yytext, NULL, radix),

	// 	// .ts = ts,

	// 	// TODO: remove these
	// 	.sign = sign,
	// 	.type = type
	// };
}

union astnode *make_one(void)
{
	char *tmp = yytext;
	union astnode *one;

	yytext = "1";
	one = make_int(10, BT_INT, LLS_UNSPEC, SIGN_SIGNED);
	yytext = tmp;

	return one;
}

// struct number make_fp(enum type type) {
union astnode *make_fp(enum scalar_basetype type, enum scalar_lls lls)
{
	union astnode *number, *ts;

	// alloc and set union astnode type representation
	ALLOC_TYPE(ts, NT_TS_SCALAR);
	ts->ts_scalar.basetype = type;
	ts->ts_scalar.modifiers.lls = lls;

	ALLOC_TYPE(number, NT_NUMBER);
	number->num.ts = ts;
	*((long double*)number->num.buf) = (long double)strtod(yytext, NULL);

	return number;

	// return (struct number) {
	// 	.real_val = strtod(yytext, NULL),
	// 	.sign = SIGNED_T,
	// 	.type = type
	// };
}

// TODO: replace with print_typespec(num->ts)

// char *print_number(struct number num) {
// char *print_number(union astnode *node)
// {
// 	char *outbuf = malloc(30);
// 	int pos = 0;
// 	struct astnode_number num = node->num;
// 	enum scalar_basetype bt = num.ts->ts_scalar.basetype;

// 	if (bt == BT_INT) {
// 		pos += sprintf(outbuf, "INTEGER %lld ",
// 			*((unsigned long long*)num.buf));
// 	} else {
// 		pos += sprintf(outbuf, "REAL %Lg ", *((long double*)num.buf));
// 	}

// 	if (num.ts->ts_scalar.modifiers.sign == SIGN_UNSIGNED) {
// 		pos += sprintf(outbuf+pos, "UNSIGNED,");
// 	}

// 	switch (expression)
// 	{
// 	case /* constant-expression */:
// 		/* code */
// 		break;
	
// 	default:
// 		break;
// 	}

// 	switch (num.ts->ts_scalar.basetype) {
// 	case BT_INT: sprintf(outbuf_pos, "INT"); break;
// 	case BT_FLOAT: sprintf(outbuf_pos, "FLOAT"); break;
// 	case BT_FLOAT: sprintf(outbuf+pos, "DOUBLE"); break;
// 	}

// 	return outbuf;

// 	// if (num.type==INT_T || num.type==LONG_T || num.type==LONGLONG_T)
// 	// 	pos += sprintf(outbuf, "INTEGER %lld ", num.int_val);
// 	// else
// 	// 	pos += sprintf(outbuf, "REAL %Lg ", num.real_val);

// 	// if (num.sign == UNSIGNED_T)
// 	// 	pos += sprintf(outbuf+pos, "UNSIGNED,");

// 	// switch (num.type) {
// 	// 	case INT_T:		sprintf(outbuf+pos, "INT"); break;
// 	// 	case LONG_T:		sprintf(outbuf+pos, "LONG"); break;
// 	// 	case LONGLONG_T:	sprintf(outbuf+pos, "LONGLONG"); break;	
// 	// 	case DOUBLE_T:	  	sprintf(outbuf+pos, "DOUBLE"); break;
// 	// 	case LONGDOUBLE_T:	sprintf(outbuf+pos, "LONGDOUBLE"); break;
// 	// 	case FLOAT_T:		sprintf(outbuf+pos, "FLOAT"); break;
// 	// }

// 	return outbuf;
// }