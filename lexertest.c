#include "errorutils.h"
#include "stringutils.h"
#include "numutils.h"
#include "lex.yy.h"
#include "parser.tab.h"

int main() {
	int t;
	char *buf = NULL, chr_buf[5];

	while (t = yylex()) {

		fprintf(stdout, "%s %d ", filename, lineno);

		// single character token
		if (t < 0xFF) {
			fprintf(stdout, "%c \n", t);
			continue;
		} else {
			fprintf(stdout, "%s ", toktostr(t));
		}

		// special token types
		switch (t) {
			case NUMBER:
				buf = print_number(yylval.num);
				break;
			case IDENT:
				fprintf(stdout, "%s", yylval.ident);
				break;
			case STRING:
				buf = print_string(&yylval.string);
				destroy_string(&yylval.string);
				break;
			case CHARLIT:
				// only single-width (ASCII) characters are
				// used in test cases
				print_char(yylval.charlit.value.none, chr_buf);
				fprintf(stdout, "%s", chr_buf);
				break;
		}

		// print_number and print_string dynamically allocate a buffer
		if (buf) {
			fprintf(stdout, "%s", buf);
			free(buf);
			buf = NULL;
		}
		fprintf(stdout, "\n");
	}

	return 0;
}