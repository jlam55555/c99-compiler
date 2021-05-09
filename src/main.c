#include <stdio.h>
#include <unistd.h>
#include <lexer/numutils.h>
#include <lexer/errorutils.h>
#include <parser.tab.h>
#include <parser/scope.h>
#include <parser/decl.h>
#include <asmgen/asm.h>
#include <lex.yy.h>
#include <common.h>

static int parse_args(int argc, char **argv)
{
	int c, i;

	while ((c = getopt(argc, argv, "d:o:")) != -1) {
		switch (c) {

		// debug output file
		case 'd':
			if (!(dfp = fopen(optarg, "w+"))) {
				fprintf(stderr, "could not open debug file %s"
					" for writing: %s\n",
					optarg, strerror(errno));
				return errno;
			}
			break;

		// output file
		case 'o':
			if (!(ofp = fopen(optarg, "w+"))) {
				fprintf(dfp, "could not open output file %s"
					" for writing: %s\n",
					optarg, strerror(errno));
				return errno;
			}
			break;

		case '?':
			return 1;
		}
	}

	// remaining arguments are the input files; set these to stdin
	if (argc - optind > 1) {
		fprintf(dfp, "warning: only one source file at a time is"
			" currently supported. Only the first file will be"
			" compiled.\n");
	}

	if (optind < argc) {
		if (!(yyin = fopen(argv[optind], "r"))) {
			fprintf(dfp, "could not open input file %s for"
				" reading: %s\n",
				argv[optind], strerror(errno));
			return errno;
		}
	}

	return 0;
}

int main(int argc, char **argv)
{
#if YYDEBUG
	yydebug = 1;
#endif

	// set default file pointers
	yyin = stdin;
	dfp = stderr;
	ofp = stdout;

	// parse user arguments
	if (parse_args(argc, argv)) {
		return 1;
	}

	// create default global scope
	scope_push(0);

	// begin parsing
	yyparse();

	// after file is complete, add global vars to output
	gen_globalvar_asm(global_vars);

	// close file pointers as appropriate
	if (yyin != stdin) {
		fclose(yyin);
	}
	if (dfp != stderr) {
		fclose(stderr);
	}
	if (ofp != stdout) {
		fclose(stdout);
	}
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

	fprintf(dfp, "%s:%d: %s: %s\n", filename, yylineno,
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