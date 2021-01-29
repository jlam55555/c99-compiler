%option noyywrap
%{
#include "tokensmanual.h"

// union yylval {
// 	int i;
// }
int yylval;
%}
%%
[0-9]+		{return NUMBER;}
[ \t\n\v\f\r]+	{;}
.		{ /* TODO: handle error */ ;}
%%
int main() {
	int t;

	while (t = yylex()) {
		fprintf(stdout, "%s: ", yytext);
		switch (t) {
			case NUMBER:
				fprintf(stdout, "Number found\n");
				break;
			default:
				fprintf(stderr, "Symbol not found\n");
		}
	}

	return 0;
}