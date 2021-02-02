#include <stdio.h>
#include <stddef.h>
#include <uchar.h>

int sizes[4] = {
	sizeof(char),
	sizeof(wchar_t),
	sizeof(char16_t),
	sizeof(char32_t),
};

// testing how wide characters 
int main() {
	int c1 = '\356\402\501';
	// wchar_t c2 = L'af';
	// wchar_t c2 = L'Δ';
	// wchar_t c2 = L'\16\224';
	wchar_t c2 = L'\224';

	char16_t c3 = u'\x394';

	fprintf(stdout, "%c %lu %lu\n", c1, sizeof c1, sizeof c3);

	fprintf(stdout, "%x %x\n", (unsigned char) c1, (unsigned) c3);

	fprintf(stdout, "%d %d %d %d\n", sizes[0], sizes[1], sizes[2], sizes[3]);

	fprintf(stdout, "%c\n", '');

	return 0;
}