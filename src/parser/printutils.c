#include <stdio.h>
#include <parser/scope.h>
#include <lexer/errorutils.h>
#include <parser/astnode.h>
#include <parser/structunion.h>
#include <parser/printutils.h>
#include <parser.tab.h>

void print_typespec(union astnode *node)
{
	FILE *fp = stdout;
	struct astnode_typespec_scalar *sc;
	struct astnode_typespec_structunion *su;

	if (!node) {
		fprintf(fp, "<unspecified type>\n");
		return;
	}

	switch (NT(node)) {

	// scalar types
	case NT_TS_SCALAR:
		sc = &node->ts_scalar;

		// signedness (if specified)
		switch (sc->modifiers.sign) {
		case SIGN_SIGNED:	fprintf(fp, "signed "); break;
		case SIGN_UNSIGNED:	fprintf(fp, "unsigned "); break;
		}

		// long/long long/short (for applicable types)
		switch (sc->modifiers.lls) {
		case LLS_SHORT:		fprintf(fp, "short "); break;
		case LLS_LONG:		fprintf(fp, "long "); break;
		case LLS_LONG_LONG:	fprintf(fp, "long long "); break;
		}

		// "base" scalar type
		switch (sc->basetype) {
		case BT_INT: 		fprintf(fp, "int"); break;
		case BT_FLOAT: 		fprintf(fp, "float"); break;
		case BT_DOUBLE: 	fprintf(fp, "double"); break;
		case BT_CHAR: 		fprintf(fp, "char"); break;
		case BT_BOOL: 		fprintf(fp, "bool"); break;

		// should only be used in function param lists, but this
		// is not really enforced
		case BT_VOID:		fprintf(fp, "void"); break;
		}
		break;

	// struct types: only need to print tag and where it was defined
	case NT_TS_STRUCT_UNION:
		su = &node->ts_structunion;
		fprintf(fp, "%s %s ",
			su->su_type == SU_STRUCT ? "struct" : "union",
			su->ident);
		if (su->is_complete) {
			fprintf(fp, "(defined at %s:%d)",
				su->def_filename, su->def_lineno);
		} else {
			fprintf(fp, "(incomplete)");
		}
		break;

	default:
		yyerror("unknown typespec");
	}
}

void print_structunion_def(union astnode *node)
{
	FILE *fp = stdout;
	union astnode *iter;
	struct astnode_typespec_structunion *su;

	su = &node->ts_structunion;

	fprintf(fp, "%s %s definition at %s:%d{\n",
		su->su_type == SU_STRUCT ? "struct" : "union",
		su->ident, su->def_filename, su->def_lineno);

	// loop through fields
	iter = su->members;
	while (iter) {
		print_symbol(iter, 0, 1);
		iter = iter->generic.next;
	}

	fprintf(fp, "}\n");
}

void print_declarator(union astnode *component, int depth)
{
	FILE *fp = stdout;
	union astnode *iter;

	// end of declarator chain
	if (!component) {
		return;
	}

	switch (NT(component)) {
	// base case
	case NT_DECL:
		print_declarator(component->decl.components, depth);
		return;

	// end of declarator chain, typespec reached
	case NT_DECLSPEC:
		print_declspec(component, depth);
		return;

	// declarator components
	case NT_DECLARATOR_ARRAY:
		INDENT(depth);
		fprintf(fp, "array (%d) of\n", *((unsigned long long*)
			component->decl_array.length->num.buf));
		break;
	case NT_DECLARATOR_POINTER:
		INDENT(depth);
		print_typequal(component->decl_pointer.spec);
		fprintf(fp, "pointer to\n");
		break;
	case NT_DECLARATOR_FUNCTION:
		INDENT(depth);
		fprintf(fp, "function\n");
		++depth;
		INDENT(depth);
		fprintf(fp, "with arguments\n");

		// print out argument list
		if (!component->decl_function.paramlist) {
			INDENT(depth+1);
			fprintf(fp, "<unknown>\n");
		} else {
			LL_FOR(component->decl_function.paramlist, iter) {
				// TODO: handle ... and void in paramlist
				print_declarator(iter, depth+1);
			}
		}

		INDENT(depth);
		fprintf(fp, "returning\n");
		break;
	default:
		fprintf(fp, "unknown type %d in print_symbol\n", NT(component));
		return;
	}

	// recursively print
	print_declarator(LL_NEXT_OF(component), depth+1);
}

void print_typequal(union astnode *node)
{
	FILE *fp = stdout;
	unsigned char tq;

	if (!node) {
		return;
	}

	tq = node->tq.qual;
	if (tq & TQ_CONST)	fprintf(fp, "const ");
	if (tq & TQ_RESTRICT)	fprintf(fp, "restrict ");
	if (tq & TQ_VOLATILE)	fprintf(fp, "volatile ");
}

void print_storageclass(union astnode *node)
{
	FILE *fp = stdout;
	enum sc_spec scspec;

	if (!node) {
		// unspecified storage class
		return;
	}

	scspec = node->sc.scspec;

	switch (scspec) {
	case SC_EXTERN:		fprintf(fp, "extern "); return;
	case SC_STATIC:		fprintf(fp, "static "); return;
	case SC_AUTO:		fprintf(fp, "auto "); return;
	case SC_REGISTER:	fprintf(fp, "register "); return;
	}
}

void print_declspec(union astnode *node, int depth)
{
	FILE *fp = stdout;

	if (!node) {
		// shouldn't happen
		yyerror("missing declspec?");
		return;
	}

	INDENT(depth);
	print_typequal(node->declspec.tq);
	print_typespec(node->declspec.ts);
	fprintf(fp, "\n");
}

void print_scope(struct scope *scope)
{
	FILE *fp = stdout;
	char *type;

	switch (scope->type) {
	case ST_FILE:		type = "global"; break;
	case ST_FUNC:		type = "function"; break;
	case ST_BLOCK:		type = "block"; break;
	case ST_PROTO:		type = "prototype"; break;
	// "fake" scope for printing purposes
	case ST_STRUCTUNION:	type = "struct/union"; break;
	default:		yyerror_fatal("unknown scope type");
	}

	fprintf(fp, "%s scope starting at %s:%d",
		type, scope->filename, scope->lineno);
}

void print_symbol(union astnode *node, int is_not_member, int depth)
{
	FILE *fp = stdout;

	if (!node || NT(node) != NT_DECL) {
		return;
	}

	INDENT(depth);
	fprintf(fp, "%s is defined ", node->decl.ident);
	if (is_not_member) {
		fprintf(fp, "with storage class ");
		print_storageclass(node->decl.declspec->declspec.sc);
	}
	fprintf(fp, "\n");
	INDENT(depth+1);
	fprintf(fp, "at %s:%d [in ", filename, lineno);
	print_scope(is_not_member ? get_scope(node->decl.ident, NS_IDENT)
		: get_current_scope());
	fprintf(fp, "] as a\n");

	// print declarator and type
	print_declarator(node->decl.components, depth+2);
}

void print_expr(union astnode *node, int depth)
{
	FILE *fp = stdout;

	if (!node) {
		return;
	}

	INDENT(depth);
	switch (node->generic.type) {

	// for symbols
	case NT_DECL:;
		int is_implicit = node->decl.is_implicit;
		fprintf(fp, "stab_%s name=%s def @%s:%d\n",
			NT(node->decl.components) == NT_DECLARATOR_FUNCTION
				? "fn" : "var",
			node->decl.ident,
			is_implicit ? "(implicit)" : node->decl.filename,
			is_implicit ? 0 : node->decl.lineno);
		break;

	// for members of structs/unions
	case NT_IDENT:
		fprintf(fp, "member %s\n", node->ident.ident);
		break;
	
	case NT_NUMBER:;
		// TODO: remove
		// char *numstring = print_number(node->num);
		enum scalar_basetype bt = node->num.ts->ts_scalar.basetype;
		enum scalar_lls lls = node->num.ts->ts_scalar.modifiers.lls;
		enum scalar_sign sign = node->num.ts->ts_scalar.modifiers.sign;

		fprintf(fp, "CONSTANT:  ");
		print_typespec(node->num.ts);
		
		if (bt == BT_INT && sign == SIGN_SIGNED) {
			fprintf(fp, "%lld\n", *((long long*)node->num.buf));
		} else if (bt == BT_INT) {
			fprintf(fp, "%llu\n",
				*((unsigned long long*)node->num.buf));
		} else {
			fprintf(fp, "%lG\n", *(long double*)node->num.buf);
		}

		// TODO: remove;
		// fprintf(fp, "CONSTANT:  %s\n", numstring);
		// free(numstring);
		break;

	case NT_STRING:;
		char *strstring = print_string(&node->string.string);
		// assumes single-width, null-terminated strings
		fprintf(fp, "STRING  %s\n", strstring);
		free(strstring);
		break;

	case NT_CHARLIT:;
		char chrstring[5];
		print_char(node->charlit.charlit.value.none, chrstring);
		// assumes single-width character literal
		fprintf(fp, "CHARLIT  %s\n", chrstring);
		break;

	case NT_BINOP:;
		int printsym = 1;
		switch (node->binop.op) {

		// these are differentiated in Hak's output
		case '=':
			fprintf(fp, "ASSIGNMENT\n");
			printsym = 0;
			break;
		case '.':
			fprintf(fp, "SELECT\n");
			printsym = 0;
			break;
		case EQEQ: case NOTEQ: case '>': case '<': case LTEQ: case GTEQ:
			fprintf(fp, "COMPARISON  OP  ");
			break;

		// cast operator
		case 'c':
			fprintf(fp, "CAST\n");
			print_declarator(node->binop.left, depth+1);
			print_expr(node->binop.right, depth+1);
			return;

		// logical operators are differentiated in hak's output
		case LOGAND: case LOGOR:
			fprintf(fp, "LOGICAL  OP  ");
			break;
		default:
			fprintf(fp, "BINARY  OP  ");
		}
		if (node->binop.op <= 0xff && printsym) {
			fprintf(fp, "%c\n", node->binop.op);
		} else if (printsym) {
			fprintf(fp, "%s\n",
				toktostr(node->binop.op));
		}
		print_expr(node->binop.left, depth+1);
		print_expr(node->binop.right, depth+1);
		break;

	case NT_UNOP:
		switch (node->unop.op) {

		// these are differentiated in Hak's output
		case '*':
			fprintf(fp, "DEREF\n");
			break;
		case '&':
			fprintf(fp, "ADDRESSOF\n");
			break;
		case SIZEOF:
			fprintf(fp, "SIZEOF\n");
			break;

		// special operator
		case 's':	// sizeof with abstract argument
			fprintf(fp, "SIZEOF (abstract)\n");
			print_declarator(node->unop.arg, depth+1);
			return;

		// ++ and -- are special cases: prefix forms
		// get rewritten, so these are specifically
		// postfix forms
		case PLUSPLUS: case MINUSMINUS:
			fprintf(fp, "UNARY  OP  POST%s\n",
				node->unop.op==PLUSPLUS ? "INC" : "DEC");
			break;
		default:
			fprintf(fp, "UNARY  OP  ");
			if (node->unop.op <= 0xff) {
				fprintf(fp, "%c\n", node->unop.op);
			} else {
				fprintf(fp, "%s\n",
					toktostr(node->unop.op));
			}
		}
		print_expr(node->unop.arg, depth+1);
		break;

	case NT_TERNOP:
		fprintf(fp, "TERNARY  OP,  IF:\n");
		print_expr(node->ternop.first, depth+1);
		INDENT(depth);
		fprintf(fp, "THEN:\n");
		print_expr(node->ternop.second, depth+1);
		INDENT(depth);
		fprintf(fp, "ELSE:\n");
		print_expr(node->ternop.third, depth+1);
		break;

	case NT_FNCALL:;
		// count number of arguments
		int argc = 0;
		union astnode *argnode = node->fncall.arglist;
		while (argnode) {
			++argc;
			argnode = argnode->generic.next;
		}
		fprintf(fp, "FNCALL,  %d  arguments\n", argc);

		// print function declarator
		print_expr(node->fncall.fnname, depth+1);

		// print arglist
		for (argc=0, argnode=node->fncall.arglist; argnode;
			++argc, argnode = argnode->generic.next) {
			INDENT(depth);
			fprintf(fp, "arg  #%d=\n", argc+1);
			print_expr(argnode, depth+1);
		}
		break;
	}
}

void print_stmt(union astnode *node, int depth)
{
	FILE *fp = stdout;
	union astnode *iter;

	if (!node) {
		return;
	}

	switch(node->generic.type) {
	case NT_STMT_EXPR:
		INDENT(depth);
		fprintf(fp, "EXPR:\n");

		print_expr(node->stmt_expr.expr, depth+1);
		break;

	case NT_STMT_LABEL:
		switch(node->stmt_label.label_type)
		{
			case LABEL_NAMED:
				INDENT(depth);
				fprintf(fp, "LABEL(%s):\n", node->stmt_label.label);
				print_stmt(node->stmt_label.body, depth+1);
				break;
			case LABEL_CASE:
				INDENT(depth);
				fprintf(fp, "CASE \n");
				INDENT(depth);
				fprintf(fp, "EXPR:\n");
				print_expr(node->stmt_label.expr, depth+1);
				print_stmt(node->stmt_label.body, depth+1);
				break;
			case LABEL_DEFAULT:
				INDENT(depth);
				fprintf(fp, "DEFAULT:\n");
				print_stmt(node->stmt_label.body, depth+1);
				break;
		}
		break;


	case NT_STMT_COMPOUND:
		INDENT(depth);
		fprintf(fp, "BLOCK {\n");

		LL_FOR(node->stmt_compound.body, iter) {
			print_stmt(iter, depth+1);
		}

		INDENT(depth);
		fprintf(fp, "}\n");
		break;

	case NT_STMT_IFELSE:
		INDENT(depth);
		fprintf(fp, "IF:\n");
		print_expr(node->stmt_if_else.cond, depth+1);

		INDENT(depth)
		fprintf(fp, "THEN:\n");
		print_stmt(node->stmt_if_else.ifstmt, depth+1);

		if(node->stmt_if_else.elsestmt)
		{
			INDENT(depth);
			fprintf(fp, "ELSE:\n");
			print_stmt(node->stmt_if_else.elsestmt, depth+1);
		}

		break;
	case NT_STMT_SWITCH:
		INDENT(depth);
		fprintf(fp, "SWITCH, EXPR:\n");
		print_expr(node->stmt_switch.cond, depth+1);

		INDENT(depth);
		fprintf(fp, "BODY:\n");
		print_stmt(node->stmt_switch.body, depth+1);

		break;

	case NT_STMT_DO_WHILE:
		INDENT(depth);
		fprintf(fp, "DO-WHILE\n");
		
		INDENT(depth);
		fprintf(fp, "BODY:\n");
		print_stmt(node->stmt_do_while.body, depth+1);

		INDENT(depth);
		fprintf(fp, "COND\n");
		print_expr(node->stmt_do_while.cond, depth+1);

		break;

	case NT_STMT_WHILE:
		INDENT(depth);
		fprintf(fp, "WHILE\n");
		INDENT(depth);
		fprintf(fp, "COND\n");
		print_expr(node->stmt_while.cond, depth+1);
		
		INDENT(depth);
		fprintf(fp, "BODY:\n");
		print_stmt(node->stmt_while.body, depth+1);
		break;

	case NT_STMT_FOR:
		INDENT(depth);
		fprintf(fp, "FOR\n");
		INDENT(depth);
		fprintf(fp, "INIT:\n");
		if (node->stmt_for.init == NULL)
			print_declarator(node->stmt_for.init, depth+1);
		else 
			print_expr(node->stmt_for.init, depth+1);

		INDENT(depth)
		fprintf(fp, "COND:\n");
		print_expr(node->stmt_for.cond, depth+1);

		INDENT(depth)
		fprintf(fp, "BODY:\n");
		print_stmt(node->stmt_for.body, depth+1);

		INDENT(depth)
		fprintf(fp, "INCR:\n");
		print_expr(node->stmt_for.update, depth+1);
		
		break;

	case NT_STMT_GOTO:
		INDENT(depth);
		fprintf(fp, "GOTO %s\n", node->stmt_goto.label);
		break;

	case NT_STMT_CONT:
		INDENT(depth);
		fprintf(fp, "CONTINUE\n");
		break;

	case NT_STMT_BREAK:
		INDENT(depth);
		fprintf(fp, "BREAK\n");
		break;
		
	case NT_STMT_RETURN:
		INDENT(depth);
		fprintf(fp, "RETURN\n");
		print_expr(node->stmt_return.rt, depth+1);
		break;
	}
	
}

// TODO: rename this print_fnbody
void print_astnode(union astnode *node)
{
	FILE *fp = stdout;

	// top-level only has declarators and function defs (which are also
	// implemented as declarators with bodies). Declarations are already
	// printed as they are occur, so just print function bodies

	// Declarators don't return anything, so this can be used to filter out
	// the top-level nodes
	if (node) {
		fprintf(fp, "AST Dump for function\n");
		print_stmt(node->decl.fn_body, 0);
	}
}