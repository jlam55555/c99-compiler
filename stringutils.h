#ifndef STRINGUTILSH
#define STRINGUTILSH

// begin building a string or (potentially wide) character constant in memory
void begin_string(char *yytext, int isstr);

// finish building the string, returns the built string
char *end_string();

// appends a simple string to the current string being built
void append_text(char *yytext);

// parses an escape sequence and appends it to the current string being built
void parse_append_escape(char *yytext);
void parse_append_octal(char *yytext);
void parse_append_hexadecimal(char *yytext);

#endif // STRINGUTILSH