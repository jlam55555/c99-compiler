#ifndef STRINGUTILSH
#define STRINGUTILSH

// this struct allows for arbitrary-length buffers. The buffer will still be
// null-terminated; the length field is only if we want to print out the string,
// which will correctly print out strings with null characters
struct string {
	unsigned length;
	char *buf;
};

// helper to print a struct string, showing escape sequences
// (including null characters); prints non-printable characters in octal
void print_string(struct string *);

// begin building a string or (potentially wide) character constant in memory
// isstr=1 if string literal, isstr=0 for character constant
void begin_string(int isstr);

// finish building the string, returns the built string
struct string end_string();

// appends a simple string to the current string being built
void append_text();

// parses an escape sequence and appends it to the current string being built
void parse_append_escape();
void parse_append_octal();
void parse_append_hexadecimal();

// helper function; frees the pointer of a struct string
void destroy_string(struct string *);

#endif // STRINGUTILSH