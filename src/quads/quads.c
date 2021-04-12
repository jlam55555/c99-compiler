#include <quads/quads.h>
#include <string.h>

// current function name and basic block number
static char *fn_name;
static int bb_no;

static struct basic_block *basic_block_new()
{
	struct basic_block *bb = calloc(1, sizeof(struct basic_block));

	// set basic block identifier
	bb->fn_name = strdup(fn_name);
	bb->bb_no = bb_no++;

	return bb;
}

/**
 * iteratively and recursively generates a linked-list of basic blocks and quads
 *
 * @param stmt		linked list of statements to generate quads for
 * @param bb		current basic block
 */
static void generate_quads_rec(union astnode *stmt, struct basic_block *bb)
{
	struct basic_block *new_bb;

	// end of recursion
	if (!stmt) {
		return;
	}

	// error
	if (!bb) {
		yyerror_fatal("compiler: bb bb cannot be NULL");
		return;
	}

	switch (NT(stmt)) {

	// compound statement: continue generating quads from body
	case NT_STMT_COMPOUND:
		generate_quads_rec(stmt->stmt_compound.body, bb);
		break;

	// expression statement: break down into subexpressions
	case NT_STMT_EXPR:
		NYI("expression statement quad generation");
		break;

	// label statements: declare a new bb
	case NT_STMT_LABEL:
	case NT_STMT_CASE:
		new_bb = basic_block_new();

		// TODO: associate label/case astnode with this bb

		bb->next_def = new_bb;
		bb = new_bb;
		break;

	// unconditional jump statements; terminate current basic block
	// (but have to keep going in case of labels further on)
	case NT_STMT_RETURN:
	case NT_STMT_CONT:
	case NT_STMT_BREAK:
	case NT_STMT_GOTO:
		NYI("unconditional jump statement quad generation");
		break;

	// conditional jump statement
	// TODO: also ternary?
	case NT_STMT_IFELSE:
		NYI("conditional jump statement quad generation");
		break;

	// loops
	case NT_STMT_FOR:
	case NT_STMT_WHILE:
	case NT_STMT_DO_WHILE:
		NYI("loop quad generation");
		break;

	default:
		// TODO: are any statement types missed?
		NYI("other stmt types");
	}

	// iterate
	generate_quads_rec(LL_NEXT(stmt), bb);
}

struct basic_block *generate_quads(union astnode *fn_decl)
{
	struct basic_block *fn_bb;

	// new function was just declared, update identifiers
	fn_name = strdup(fn_decl->decl.ident);
	bb_no = 0;

	// create starting basic block of function
	fn_bb = basic_block_new();

	// recursively generate quads for each statement
	generate_quads_rec(fn_decl->decl.fn_body, fn_bb);

	return fn_bb;
}

void print_basic_block(struct basic_block *bb)
{
	// TODO
}