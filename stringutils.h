#ifndef STRINGUTILSH
#define STRINGUTILSH

#include <stddef.h>

// macros from https://stackoverflow.com/a/5920028/2397327
#ifdef __APPLE__
	typedef uint_least16_t char16_t;
	typedef uint_least32_t char32_t;
#elif __linux__
	#include <uchar.h>
#endif

enum literal_type {
	LT_STRING,
	LT_CHARLIT
};

// allows C11 char types
enum char_width {
	CW_NONE = 0,
	CW_L,
	CW_u,
	CW_U,
	CW_u8	// for strings only
};
extern size_t char_widths[5];

// more generic char type
union char_t {
	char none;
	wchar_t L;
	char16_t u;
	char32_t U;
	unsigned char u8;
};

// this struct allows for arbitrary-length buffers. The buffer will still be
// null-terminated; the length field is only if we want to print out the string,
// which will correctly print out strings with null characters
struct string {
	unsigned length;
	enum char_width width;
	void *buf;
};

struct charlit {
	enum char_width width;
	union char_t value;
};

// begin building a string or (potentially wide) character constant in memory
// isstr=1 if string literal, isstr=0 for character constant
void begin_literal();

// finish building the string, returns the built string
struct string end_string();
struct charlit end_charlit();

// appends a simple string to the current string being built
void append_text();

// parses an escape sequence and appends it to the current string being built
void parse_append_escape();
void parse_append_octal();
void parse_append_hexadecimal();

// helper to print a single byte, showing escape sequences;
// prints to the buffer, which should be at least 5 bytes long
void print_char(unsigned char value, char *buf);

// helper to print a struct string, showing escape sequences
// (including null characters); prints non-printable characters in octal;
// returns a dynamically-allocated string that should be fred
char *print_string(struct string *);

// helper function; frees the pointer of a struct string
void destroy_string(struct string *);

#endif // STRINGUTILSH