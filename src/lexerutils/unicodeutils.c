#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexerutils/unicodeutils.h"

// detects unicode characters by looking at the first bits of
// each byte; see https://stackoverflow.com/a/1543616/2397327
// this assumes that utf8_text is a valid C string, i.e., null-terminated;
// this assumes that utf8_text is also a valid UTF-8 string;
// returns length of created buffer, or -1 on error;
// buffer should be manually freed when finished using it
size_t utowc(char *utf8_text, size_t char_width, void **buf) {
	char *c = utf8_text;
	unsigned long value, width, buf_len = 16, cur_len = 0;
	void *strbuf = malloc(buf_len);

	while (*c) {
		// detect width of current code point
		if (!(*c & 0x80)) {
			width = 1;
		} else if (!(*c & 0x20)) {
			width = 2;
		} else if (!(*c & 0x10)) {
			width = 3;
		} else if (!(*c & 0x08)) {
			width = 4;
		} else {
			fprintf(stderr, "Error: Invalid UTF-8 byte.\n");
			return -1;
		}

		if (width > char_width) {
			fprintf(stderr, "Warning: UTF-8 character exceeds "
				"character width.\n");
		}

		// realloc if necessary
		if (char_width * (cur_len+1) < buf_len) {
			strbuf = realloc(strbuf, buf_len *= 2);
		}

		// copy exactly one code point and truncate if
		// width exceeds char_width
		value = 0;
		memcpy(&value, c, width);
		memcpy(strbuf+char_width*cur_len, &value, char_width);

		++cur_len;
		c += width;
	}

	*buf = strbuf;
	return char_width * cur_len;

fail:
	fprintf(stderr, "UTF8 code point.\n");
	return -1;
}