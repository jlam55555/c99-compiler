#ifndef ERRORUTILSH
#define ERRORUTILSH

extern int lineno;
extern char filename[255];

// Simple function to parse preprocessor directive generated by
// `gcc -E`. Note that this implementation (and the associated regex)
// don't account for escaped quotes in the filename.
// (Who puts quotes in filenames?)
void parse_lineno();

// Printing lexical errors with their context (i.e., with filename, lineno).
// This is probably called once per start condition
// (start condition ~ environment).
void print_lexical_error();

// Prints the name of the token type.
char *toktostr(int enumval);

#endif	// ERRORUTILSH