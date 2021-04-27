#include <quads/cfquads.h>
#include <quads/quads.h>
#include <quads/exprquads.h>
#include <parser.tab.h>
#include <malloc.h>

// used to hold state information about the current loops; information for
// parent loops is stored on the stack frame
static struct loop *cur_loop;

struct loop *loop_new(void)
{
	struct loop *lp = malloc(sizeof(struct loop));
	lp->prev = cur_loop;
	return lp;
}

void generate_for_quads(union astnode *stmt, struct basic_block *bb)
{
	struct basic_block *bb_cond = basic_block_new();
	struct basic_block *bb_body = basic_block_new();
	struct basic_block *bb_update = basic_block_new();
	struct basic_block *bb_next = basic_block_new();

	cur_loop = loop_new();
	// Continue and break points
	cur_loop->bb_cont = bb_cond;
	cur_loop->bb_break = bb_next;

	// gen_assign(stmt->stmt_for.init);
}
void generate_do_while_quads(union astnode *stmt, struct basic_block *bb)
{
	struct basic_block *bb_cond = basic_block_new();
	struct basic_block *bb_body = basic_block_new();
	struct basic_block *bb_next = basic_block_new();

	cur_loop = loop_new();
	// Continue and break points
	cur_loop->bb_cont = bb_cond;
	cur_loop->bb_break = bb_next;

	link_basic_block(bb, ALWAYS, bb_body, NULL);
	bb = bb_body;

	generate_quads_rec(stmt->stmt_while.body, bb);

	link_basic_block(bb, ALWAYS, bb_cond, NULL);
	bb = bb_cond;

	generate_conditional_quads(stmt->stmt_while.cond, bb, bb_body, bb_next);

	bb = bb_next;
	cur_loop = cur_loop->prev;
}

void generate_while_quads(union astnode *stmt, struct basic_block *bb)
{
	struct basic_block *bb_cond = basic_block_new();
	struct basic_block *bb_body = basic_block_new();
	struct basic_block *bb_next = basic_block_new();

	cur_loop = loop_new();
	// Continue and break points
	cur_loop->bb_cont = bb_cond;
	cur_loop->bb_break = bb_next;

	link_basic_block(bb, ALWAYS, bb_cond, NULL);
	bb = bb_cond;

	generate_conditional_quads(stmt->stmt_while.cond, bb, bb_body, bb_next);

	bb = bb_body;
	generate_quads_rec(stmt->stmt_while.body, bb);

	link_basic_block(bb, ALWAYS, bb_cond, NULL);

	bb = bb_next;
	cur_loop = cur_loop->prev;
}

void generate_if_else_quads(union astnode *expr, struct basic_block *bb)
{
	struct basic_block *Bt = basic_block_new();
	struct basic_block *Bf = basic_block_new();
	struct basic_block *Bn;

	if (expr->stmt_if_else.elsestmt) {
		Bn = basic_block_new();
	} else {
		Bn = Bf;
	}

	generate_conditional_quads(expr->stmt_if_else.ifstmt, bb, Bt, Bf);

	bb = Bt;

	generate_quads_rec(expr->stmt_if_else.ifstmt, bb);
	link_basic_block(bb, ALWAYS, Bn, NULL);

	if (expr->stmt_if_else.elsestmt) {
		bb = Bf;
		generate_quads_rec(expr->stmt_if_else.elsestmt, bb);
		link_basic_block(bb, ALWAYS, Bn, NULL);
	}

	bb = Bn;
}

void generate_conditional_quads(union astnode *expr, struct basic_block *bb,
	struct basic_block *Bt, struct basic_block *Bf)
{
	switch(expr->generic.type) {
	case NT_BINOP:;
		struct addr *addr1 = gen_rvalue(expr->binop.left, NULL, bb);
		struct addr *addr2 = gen_rvalue(expr->binop.right, NULL, bb);
		quad_new(bb, OC_CMP, NULL, addr1, addr2);

		// struct addr *addr_bt = gen
		switch(expr->binop.op) {
		case '<':	link_basic_block(bb, BR_LT, Bt, Bf);	break;
		case '>':	link_basic_block(bb, BR_GT, Bt, Bf);	break;
		case LTEQ:	link_basic_block(bb, BR_LTEQ, Bt, Bf);	break;
		case GTEQ:	link_basic_block(bb, BR_GTEQ, Bt, Bf);	break;
		case EQEQ:	link_basic_block(bb, BR_EQ, Bt, Bf);	break;
		case NOTEQ:	link_basic_block(bb, BR_NEQ, Bt, Bf);	break;
		default:	yyerror("binop in conditional expr\n");
		}
		break;
	}
}

void generate_cont_break_quads(union astnode *stmt, struct basic_block *bb)
{
	if (NT(stmt)) {
		link_basic_block(bb, ALWAYS, cur_loop->bb_break, NULL);
	} else {
		if(NT(stmt)) {
			link_basic_block(bb, ALWAYS, cur_loop->bb_cont, NULL);
		}
	}
}


struct basic_block *link_basic_block(struct basic_block *bb,
	enum branches branch, struct basic_block *prev,
	struct basic_block *next)
{
	bb->branch = branch;
	bb->prev = prev;
	bb->next_cond = next;

	return bb;
}