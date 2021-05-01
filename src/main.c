#include <stdio.h>
#include <unistd.h>
#include <lexer/numutils.h>
#include <lexer/stringutils.h>
#include <lexer/errorutils.h>
#include <parser/parser.h>
#include <parser.tab.h>
#include <parser/astnode.h>
#include <parser/scope.h>

int main(void)
{
	#if YYDEBUG
	yydebug = 1;
	#endif

	// create default global scope
	scope_push(0);

	// begin parsing
	yyparse();
}

// declared in parser.h
static int is_fatal_error = 0;
int yyerror(const char *err)
{
	char buf[1024];

	// replace default syntax error message
	if (!strcmp(err, "syntax error")) {
		is_fatal_error = 1;
		snprintf(buf, sizeof(buf), "unexpected token \"%s\"\n", yytext);
		err = buf;
	}

	fprintf(stderr, "%s:%d: %s: %s\n", filename, yylineno,
		is_fatal_error ? "error" : "warning", err);

	if (is_fatal_error) {
		_exit(-1);
	}
}

// declared in parser.h
int yyerror_fatal(const char *err)
{
	is_fatal_error = 1;
	yyerror(err);
}