#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "stringutils.h"

// TODO: need to differentiate between characters
// TODO: watch for overflow with octal/hex constants
// TODO: implement wide chars

extern char *yytext;

// TODO: this should probably return a regular C string rather than
// print to stdout (e.g., in the case of needing to print to stderr)
void print_string(struct string *str) {
	// necessary to iterate over length, since there may be null
	// characters within the string
	unsigned i;
	unsigned char *c, *to_print;

	for (i = 0, c = str->buf; i < str->length; i++, c++) {
		// printable character: print literally
		if (isprint(*c)) {
			switch (*c) {
				case '\\': case '\'': case '"':
					fprintf(stdout, "\\%c", *c);
					continue;
				default:
					fprintf(stdout, "%c", *c);
					continue;
			}
		}

		// non-printable characters
		switch (*c) {
			case '\0': to_print = "\\0"; break;
			case '\a': to_print = "\\a"; break;
			case '\b': to_print = "\\b"; break;
			case '\f': to_print = "\\f"; break;
			case '\n': to_print = "\\n"; break;
			case '\r': to_print = "\\r"; break;
			case '\t': to_print = "\\t"; break;
			case '\v': to_print = "\\v"; break;

			// non-common escape code: print octal code
			default:
				fprintf(stdout, "\\%03o", *c);
				continue;
		}

		// common escape code: print escape code
		fprintf(stdout, to_print);
	}
}

// for building the string; buf_len includes the null character at the
// end, cur_len does not
static char *strbuf = NULL;
static unsigned cur_len, buf_len, initial_size = 16;

// helper function to append chars and dynamically allocate memory as needed
static void append_buffer(char *append_buf, unsigned append_buf_len) {
	// realloc when necessary; doubles buffer size until sufficient
	if (buf_len - cur_len - 1 < append_buf_len) {
		while (buf_len - cur_len - 1 < append_buf_len)
			buf_len <<= 1;
		strbuf = (char *) realloc(strbuf, buf_len);
	}

	memcpy(strbuf + cur_len, append_buf, append_buf_len);
	cur_len += append_buf_len;
}

void begin_string(int isstr) {
	unsigned len = strlen(yytext);

	// yytext len > 1 <=> wide characters
	// TODO: wide chars not implemented (yet?)
	if (len > 1) {
		fprintf(stderr, "Error: wide characters not implemented");
		return;
	}

	strbuf = (char *) malloc(initial_size * sizeof(char));
	buf_len = initial_size;
	cur_len = 0;
}

struct string end_string() {
	strbuf[cur_len] = 0;
	struct string str = {
		.length = cur_len,
		.buf = realloc(strbuf, cur_len + 1)
	};
	return str;
}

void append_text() {
	unsigned len = strlen(yytext);
	append_buffer(yytext, len);
}

void parse_append_escape() {
	char val;

	// TODO: should move this to a map using an array
	switch (yytext[1]) {
		case '\\':
		case '\'':
		case '"':
		case '?':
			val = yytext[1];
			break;
		case 'a': val = '\a'; break;
		case 'b': val = '\b'; break;
		case 'f': val = '\f'; break;
		case 'n': val = '\n'; break;
		case 'r': val = '\r'; break;
		case 't': val = '\t'; break;
		case 'v': val = '\v'; break;
		default:
			// TODO: need better error handling
			fprintf(stderr, "Error: bad escape code %c\n",
				yytext[1]);
			return;
	}
	append_buffer(&val, 1);
}

void parse_append_octal() {
	unsigned char val = 0, *it = yytext;

	// skip over leading slash
	while (*++it) {
		val = (val<<3) + (*it-'0');
	}

	append_buffer(&val, 1);

	// TODO: handle overflow
}

// helper function to convert hexidecimal to decimal;
// probably faster to implement this as a map but this is fine
static unsigned char htod(char c) {
	if (c >= '0' && c <= '9') {
		return c - '0';
	} else if (c >= 'A' && c <= 'F') {
		return 10 + c - 'A';
	} else if (c >= 'a' && c <= 'f') {
		return 10 + c - 'a';
	} else {
		fprintf(stderr, "Error: invalid hexidecimal character");
		return 0;
	}
}

void parse_append_hexadecimal() {
	unsigned char val = 0, *it = yytext + 1;

	// skip over leading \x
	while (*++it) {
		val = (val<<4) + htod(*it);
	}
	append_buffer(&val, 1);

	// TODO: handle overflow
}

void destroy_string(struct string *str) {
	if (str->buf) {
		free(str->buf);
	}
}