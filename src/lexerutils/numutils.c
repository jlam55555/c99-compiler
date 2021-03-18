#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "lexerutils/numutils.h"

extern char *yytext;

struct number make_int(int radix, enum sign sign, enum type type) {
	return (struct number) {
		.int_val = strtoull(yytext, NULL, radix),
		.sign = sign,
		.type = type
	};
}

struct number make_fp(enum type type) {
	return (struct number) {
		.real_val = strtod(yytext, NULL),
		.sign = SIGNED_T,
		.type = type
	};
}

char *print_number(struct number num) {
	char *outbuf = malloc(30);
	int pos = 0;

	if (num.type==INT_T || num.type==LONG_T || num.type==LONGLONG_T)
		pos += sprintf(outbuf, "INTEGER %lld ", num.int_val);
	else
		pos += sprintf(outbuf, "REAL %Lg ", num.real_val);

	if (num.sign == UNSIGNED_T)
		pos += sprintf(outbuf+pos, "UNSIGNED,");

	switch (num.type) {
		case INT_T:		sprintf(outbuf+pos, "INT"); break;
		case LONG_T:		sprintf(outbuf+pos, "LONG"); break;
		case LONGLONG_T:	sprintf(outbuf+pos, "LONGLONG"); break;	
		case DOUBLE_T:	  	sprintf(outbuf+pos, "DOUBLE"); break;
		case LONGDOUBLE_T:	sprintf(outbuf+pos, "LONGDOUBLE"); break;
		case FLOAT_T:		sprintf(outbuf+pos, "FLOAT"); break;
	}

	return outbuf;
}