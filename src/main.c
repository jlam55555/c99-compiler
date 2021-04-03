#include <stdio.h>
#include <unistd.h>
#include "lexerutils/numutils.h"
#include "lexerutils/stringutils.h"
#include "lexerutils/errorutils.h"
#include "parser.h"
#include "parser.tab.h"
#include "astnode.h"
#include "scope.h"

int main()
{
	#if YYDEBUG
	yydebug = 1;
	#endif

	// create default global scope
	scope_push(0);

	// begin parsing
	yyparse();
}

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

int yyerror_fatal(const char *err)
{
	is_fatal_error = 1;
	yyerror(err);
}

// TODO: move this function to printutils
void print_astnode_rec(union astnode *node, int depth)
{
	INDENT(depth);
	switch (node->generic.type) {
	case NT_NUMBER:;
		char *numstring = print_number(node->num.num);
		fprintf(stdout, "CONSTANT:  %s\n", numstring);
		free(numstring);
		break;
	case NT_STRING:;
		char *strstring = print_string(&node->string.string);
		// assumes single-width, null-terminated strings
		fprintf(stdout, "STRING  %s\n", strstring);
		free(strstring);
		break;
	case NT_CHARLIT:;
		char chrstring[5];
		print_char(node->charlit.charlit.value.none, chrstring);
		// assumes single-width character literal
		fprintf(stdout, "CHARLIT  %s\n", chrstring);
		break;
	case NT_IDENT:
		fprintf(stdout, "IDENT  %s\n", node->ident.ident);
		break;
	case NT_BINOP:;
		int printsym = 1;
		switch (node->binop.op) {
		// these are differentiated in Hak's output
		case '=':
			fprintf(stdout, "ASSIGNMENT\n");
			printsym = 0;
			break;
		case '.':
			fprintf(stdout, "SELECT\n");
			printsym = 0;
			break;
		case EQEQ: case NOTEQ: case '>': case '<': case LTEQ: case GTEQ:
			fprintf(stdout, "COMPARISON  OP  ");
			break;
		// logical operators are differentiated in hak's output
		case LOGAND: case LOGOR:
			fprintf(stdout, "LOGICAL  OP  ");
			break;
		default:
			fprintf(stdout, "BINARY  OP  ");
		}
		if (node->binop.op <= 0xff && printsym) {
			fprintf(stdout, "%c\n", node->binop.op);
		} else if (printsym) {
			fprintf(stdout, "%s\n",
				toktostr(node->binop.op));
		}
		print_astnode_rec(node->binop.left, depth+1);
		print_astnode_rec(node->binop.right, depth+1);
		break;
	case NT_UNOP:
		switch (node->unop.op) {
		// these are differentiated in Hak's output
		case '*':
			fprintf(stdout, "DEREF\n");
			break;
		case '&':
			fprintf(stdout, "ADDRESSOF\n");
			break;
		case SIZEOF:
			fprintf(stdout, "SIZEOF\n");
			break;
		// ++ and -- are special cases: prefix forms
		// get rewritten, so these are specifically
		// postfix forms
		case PLUSPLUS: case MINUSMINUS:
			fprintf(stdout, "UNARY  OP  POST%s\n",
				node->unop.op==PLUSPLUS ? "INC" : "DEC");
			break;
		default:
			fprintf(stdout, "UNARY  OP  ");
			if (node->unop.op <= 0xff) {
				fprintf(stdout, "%c\n", node->unop.op);
			} else {
				fprintf(stdout, "%s\n",
					toktostr(node->unop.op));
			}
		}
		print_astnode_rec(node->unop.arg, depth+1);
		break;
	case NT_TERNOP:
		fprintf(stdout, "TERNARY  OP,  IF:\n");
		print_astnode_rec(node->ternop.first, depth+1);
		INDENT(depth);
		fprintf(stdout, "THEN:\n");
		print_astnode_rec(node->ternop.second, depth+1);
		INDENT(depth);
		fprintf(stdout, "ELSE:\n");
		print_astnode_rec(node->ternop.third, depth+1);
		break;
	case NT_FNCALL:;
		// count number of arguments
		int argc = 0;
		union astnode *argnode = node->fncall.arglist;
		while (argnode) {
			++argc;
			argnode = argnode->generic.next;
		}
		fprintf(stdout, "FNCALL,  %d  arguments\n", argc);

		// print function declarator
		print_astnode_rec(node->fncall.fnname, depth+1);

		// print arglist
		for (argc=0, argnode=node->fncall.arglist; argnode;
			++argc, argnode = argnode->generic.next) {
			INDENT(depth);
			fprintf(stdout, "arg  #%d=\n", argc+1);
			print_astnode_rec(argnode, depth+1);
		}
		break;
	default:
		fprintf(stdout, "AST type %d not implemented yet.\n",
			node->generic.type);
	}
}

void print_astnode(union astnode *node)
{
	print_astnode_rec(node, 0);
	fprintf(stdout, "\n");
}