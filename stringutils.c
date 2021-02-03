#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "stringutils.h"
#include "unicodeutils.h"

// TODO: need to differentiate between characters
// TODO: watch for overflow with octal/hex constants
// TODO: implement wide chars

extern char *yytext;

size_t char_widths[5] = {
	sizeof(char),			// CW_NONE
	sizeof(wchar_t),		// CW_L
	sizeof(char16_t),		// CW_u
	sizeof(char32_t),		// CW_U
	sizeof(unsigned char)		// CW_u8
};

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
enum literal_type literal_type;
enum char_width char_width;
union char_t char_val;

void begin_literal() {
	unsigned len = strlen(yytext);

	// detecting literal type
	literal_type = yytext[len-1] == '"' ? LT_STRING : LT_CHARLIT;

	// detecting character width
	switch (yytext[0]) {
		case '\'': case '\"':
			char_width = CW_NONE;
			break;
		case 'U':
			char_width = CW_U;
			break;
		case 'L':
			char_width = CW_L;
			break;
		case 'u':
			char_width = yytext[1] == '8' ? CW_u8 : CW_u;
			break;
		default:
			fprintf(stderr, "Error: unknown char width");
			return;
	}

	// initialize empty string/character
	if (literal_type == LT_STRING) {
		strbuf = (void *) malloc(initial_size * sizeof(char));
		buf_len = initial_size;
		cur_len = 0;
	} else {
		// cur_len to indicate whether character is filled or not
		// in the case of multi-character literals (which are
		// not supported in this implementation); also used to
		// indicate empty character constant (an error)
		cur_len = 0;
	}
}

struct string end_string() {
	strbuf[cur_len] = 0;
	switch (char_width) {
		case CW_NONE:
			((char *) strbuf)[cur_len] = 0;
			break;
		case CW_L:
			((wchar_t *) strbuf)[cur_len] = 0;
			break;
		case CW_u:
			((char16_t *) strbuf)[cur_len] = 0;
			break;
		case CW_U:
			((char32_t *) strbuf)[cur_len] = 0;
			break;
		case CW_u8:
			((unsigned char *) strbuf)[cur_len] = 0;
			break;
		default:
			fprintf(stderr, "Error: unknown character width.\n");
			return (struct string) {};
	}

	return (struct string) {
		.width = char_width,
		.length = cur_len,
		.buf = realloc(strbuf, (cur_len+1) * char_widths[char_width])
	};
}

struct charlit end_charlit() {
	// empty character constant is an error
	if (!cur_len) {
		fprintf(stderr, "Error: empty character constant.\n");
		return (struct charlit) {};
	}

	// multiple code points is a warning; we mimick the behavior of gcc8
	// by returning only the last code point in the character constant
	if (cur_len > char_widths[char_width]) {
		fprintf(stderr, "Warning: multiple code points in character "
			"constant.\n");
	}

	return (struct charlit) {
		.width = char_width,
		.value = char_val
	};
}

// helper function to append chars and dynamically allocate memory as needed
// for strings, and sets the character for character literals;
// append_buf_len is the number of bytes to add to the buffer, not the
// number of characters (append_buf_length = number_of_characters*char_width
// except for u8 strings, which have a VLE)
static void append_buffer(void *append_buf, unsigned append_buf_len) {

	// strings
	if (literal_type == LT_STRING) {
		// realloc when necessary; doubles buffer size until sufficient
		if (buf_len - cur_len - 1 < append_buf_len) {
			while (buf_len - cur_len - 1 < append_buf_len)
				buf_len <<= 1;
			strbuf = realloc(strbuf, buf_len);
		}

		memcpy(strbuf + cur_len, append_buf, append_buf_len);
		cur_len += append_buf_len;
		return;
	}

	// character literal: no dynamic allocation, simply write to value;
	// this assumes that append_buf_len == 1; the end behavior is
	// consistent with the behavior of gcc8

	// this matches the behavior on gcc8: treat character constant as
	// an integer, and truncate to the first char_width bytes, assuming
	// a little-endian system
	switch (char_width) {
		case CW_NONE:
			char_val.none = *((char *) append_buf);
			break;
		case CW_L:
			char_val.L = *((wchar_t *) append_buf);
			break;
		case CW_u:
			char_val.u = *((char16_t *) append_buf);
			break;
		case CW_U:
			char_val.U = *((char32_t *) append_buf);
			break;
		case CW_u8:
			char_val.u8 = *((unsigned char *) append_buf);
			break;
		default: 
			fprintf(stderr, "Error: unknown character type\n");
			return;
	}
	cur_len += append_buf_len;
}

// this assumes a UTF-8 source file
// TODO: document this somewhere else
void append_text() {
	void *buf;
	int len;

	// for a UTF-8 string paste string literally
	if (literal_type == LT_STRING && char_width == CW_u8) {
		append_buffer(yytext, strlen(yytext));
		return;
	}

	// otherwise, convert to unicode string with fixed width
	len = utowc(yytext, char_widths[char_width], &buf);
	append_buffer(buf, len);
	free(buf);
}

void parse_append_escape() {
	unsigned long val;

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

// note that this assumes a little-endian system; unsigned
// long should be at least as long as all possible character types
// (i.e., at least 32 bits), so it acts as a simple LE-buffer
void parse_append_octal() {
	unsigned long val = 0;
	char *it = yytext;

	// detect and handle overflow; overflow is only possible if
	// 3 digits with first digit is > 3 and 1 byte width
	if (char_widths[char_width] == 1 && 
		strlen(yytext) == 4 && yytext[1] == '3') {
		fprintf(stderr, "Warning: octal escape code %s exceeds "
			"code point width.\n", yytext);
	}

	// skip over leading slash
	while (*++it)
		val = (val<<3) + (*it-'0');

	append_buffer(&val, char_widths[char_width]);
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
	// see notes for parse_append_octcal
	unsigned long val = 0;
	char *it = yytext + 1;

	// detect overflow: since each byte is 2 hex digits,
	// overflow if digits > 2 * char_width
	if (strlen(yytext) - 2 > 2 * char_widths[char_width]) {
		fprintf(stderr, "Warning: hexadecimal escape code %s exceeds "
			"code point width.\n", yytext);
	}

	// skip over leading \x
	while (*++it)
		val = (val<<4) + htod(*it);

	append_buffer(&val, 1);
}

void destroy_string(struct string *str) {
	if (str->buf) {
		free(str->buf);
	}
}