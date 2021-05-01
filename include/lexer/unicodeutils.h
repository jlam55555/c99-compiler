#ifndef UNICODEUTILSH
#define UNICODEUTILSH

#include <stddef.h>

// converts UTF-8 encoded text to an array of fixed-width characters;
// updates buf to point to new string, and returns the size (in bytes)
// of the buffer; generated string is not null-terminated and should be
// freed after use
size_t utowc(char *utf8_text, size_t char_width, void **buf);

#endif // UNICODEUTILSH