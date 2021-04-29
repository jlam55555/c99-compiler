#include <quads/quads.h>
#include <quads/printutils.h>
#include <quads/sizeof.h>
#include <quads/exprquads.h>
#include <quads/cfquads.h>
#include <string.h>

struct basic_block *cur_bb;

// current function name and basic block number
static char *fn_name;
static int bb_no, tmp_no;

struct basic_block *basic_block_new(void)
{
	struct basic_block *bb = calloc(1, sizeof(struct basic_block));

	// set basic block identifier
	bb->fn_name = strdup(fn_name);
	bb->bb_no = bb_no++;
	return bb;
}

struct basic_block *dummy_basic_block_new(void)
{
	struct basic_block *bb = calloc(1, sizeof(struct basic_block));

	bb->bb_no = -1;
	return bb;
}

struct quad *quad_new(struct basic_block *bb, enum opcode opcode,
	struct addr *dest, struct addr *src1, struct addr *src2)
{
	struct quad *quad = calloc(1, sizeof(struct quad));

	*quad = (struct quad) {
		.bb = bb,
		.next = bb->ll,

		.opcode = opcode,
		.dest = dest,
		.src1 = src1,
		.src2 = src2,
	};

	// note: this generates the basic block in reverse
	bb->ll = quad;

	return quad;
}

struct addr *addr_new(enum addr_type type, union astnode *decl)
{
	struct addr *addr = calloc(1, sizeof(struct addr));

	*addr = (struct addr) {
		.type = type,
		.size = astnode_sizeof_type(decl),
		.decl = decl,
	};

	return addr;
}

struct addr *tmp_addr_new(union astnode *decl)
{
	struct addr *addr = addr_new(AT_TMP, decl);

	// assign unique temporary (pseudo-)register identifier
	addr->val.tmpid = tmp_no++;

	return addr;
}

void generate_quads_rec(union astnode *stmt, struct basic_block *bb)
{
	struct basic_block *new_bb;

	// end of recursion
	if (!stmt) {
		return;
	}

	// error
	if (!bb) {
		yyerror_fatal("quadgen: bb bb cannot be NULL");
		return;
	}

	switch (NT(stmt)) {

	// compound statement: continue generating quads from body
	case NT_STMT_COMPOUND:
		generate_quads_rec(stmt->stmt_compound.body, bb);
		break;

	// expression statement: break down into subexpressions
	case NT_STMT_EXPR:
		// TODO: warn if no side-effects (i.e., statement is useless)

		gen_rvalue(stmt->stmt_expr.expr, NULL, bb, NULL);
		break;

	// label statements: declare a new bb
	case NT_STMT_LABEL:
		new_bb = basic_block_new();

		// TODO: associate label/case astnode with this bb

		// TODO: name basic block with the label, not a number

		bb->next_def = new_bb;
		bb = new_bb;

		// TODO: make labelled statements flat? matches syntax better
		// 	that way; this also doesn't work correctly with multiple
		// 	nested labels
//		generate_quads_rec(stmt->stmt_label.body, bb);
		break;

	// unconditional jump statements; terminate current basic block
	// (but have to keep going in case of labels further on)
	case NT_STMT_RETURN:
		break;
	case NT_STMT_CONT:
	case NT_STMT_BREAK:
		break;
	case NT_STMT_GOTO:
		NYI("unconditional jump statement quad generation");
		break;

	// conditional jump statement
	// TODO: also ternary?
	case NT_STMT_IFELSE:
		bb = generate_if_else_quads(stmt, bb);
		break;

	// loops
	case NT_STMT_FOR:
		generate_for_quads(stmt, bb);
		break;
	case NT_STMT_WHILE:
		generate_while_quads(stmt, bb);
		break;
	case NT_STMT_DO_WHILE:
		generate_do_while_quads(stmt, bb);
		break;

	default:
		// TODO: are any statement types missed?
		NYI("other stmt types quad generation");
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

#if DEBUG
	// dump basic blocks
	print_basic_blocks(fn_bb);
#endif

	return fn_bb;
}