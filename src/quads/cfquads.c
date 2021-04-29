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
	NYI("for loop quad generation");

//	struct basic_block *bb_cond = basic_block_new();
//	struct basic_block *bb_body = basic_block_new();
//	struct basic_block *bb_update = basic_block_new();
//	struct basic_block *bb_next = basic_block_new();
//
//	cur_loop = loop_new();
//	// Continue and break points
//	cur_loop->bb_cont = bb_cond;
//	cur_loop->bb_break = bb_next;
//
//	// gen_assign(stmt->stmt_for.init);
}
void generate_do_while_quads(union astnode *stmt, struct basic_block *bb)
{
	NYI("do while quad generation");

//	struct basic_block *bb_cond = basic_block_new();
//	struct basic_block *bb_body = basic_block_new();
//	struct basic_block *bb_next = basic_block_new();
//
//	cur_loop = loop_new();
//	// Continue and break points
//	cur_loop->bb_cont = bb_cond;
//	cur_loop->bb_break = bb_next;
//
//	link_basic_block(bb, ALWAYS, bb_body, NULL);
//	bb = bb_body;
//
//	generate_quads_rec(stmt->stmt_while.body, bb);
//
//	link_basic_block(bb, ALWAYS, bb_cond, NULL);
//	bb = bb_cond;
//
//	generate_conditional_quads(stmt->stmt_while.cond, bb, bb_body, bb_next);
//
//	bb = bb_next;
//	cur_loop = cur_loop->prev;
}

void generate_while_quads(union astnode *stmt, struct basic_block *bb)
{
	NYI("while quad generation");

//	struct basic_block *bb_cond = basic_block_new();
//	struct basic_block *bb_body = basic_block_new();
//	struct basic_block *bb_next = basic_block_new();
//
//	cur_loop = loop_new();
//	// Continue and break points
//	cur_loop->bb_cont = bb_cond;
//	cur_loop->bb_break = bb_next;
//
//	link_basic_block(bb, ALWAYS, bb_cond, NULL);
//	bb = bb_cond;
//
//	generate_conditional_quads(stmt->stmt_while.cond, bb, bb_body, bb_next);
//
//	bb = bb_body;
//	generate_quads_rec(stmt->stmt_while.body, bb);
//
//	link_basic_block(bb, ALWAYS, bb_cond, NULL);
//
//	bb = bb_next;
//	cur_loop = cur_loop->prev;
}

struct basic_block *generate_if_else_quads(union astnode *expr,
	struct basic_block *bb)
{
	struct basic_block *bb_true = basic_block_new();
	struct basic_block *bb_false = basic_block_new();
	struct basic_block *bb_next;

	if (expr->stmt_if_else.elsestmt) {
		bb_next = basic_block_new();
	} else {
		bb_next = bb_false;
	}

	generate_conditional_quads(expr->stmt_if_else.ifstmt, bb,
		bb_true, bb_false);

	// generate statements for true branch
	generate_quads_rec(expr->stmt_if_else.ifstmt, bb_true);

	// TODO: remove
//	link_basic_block(bb_true, ALWAYS, bb_next, NULL);
	link_bbs(bb_true, CC_ALWAYS, bb_next, NULL);

	// generate statements for else branch
	if (expr->stmt_if_else.elsestmt) {
		generate_quads_rec(expr->stmt_if_else.elsestmt, bb_false);

		// TODO: remove
//		link_basic_block(bb_false, ALWAYS, bb_next, NULL);
		link_bbs(bb_false, CC_ALWAYS, bb_next, NULL);
	}

	return bb_next;
}

void generate_conditional_quads(union astnode *expr, struct basic_block *bb,
	struct basic_block *bb_true, struct basic_block *bb_false)
{
	enum cc cc = CC_UNSPEC;
	struct addr *cond, *zero;

	cond = gen_rvalue(expr, NULL, bb, &cc);

	// if cc is not set, then create compare and set cc to CC_NE (nonzero,
	// default truthy condition)
	// note: the TEST opcode is more efficient than the CMP opcode, but we
	// use CMP for sake of simplicity
	if (cc == CC_UNSPEC) {
		zero = addr_new(AT_CONST, create_size_t());
		*((uint64_t*)zero->val.constval) = 0;

		quad_new(bb, OC_CMP, NULL, cond, zero);
		cc = CC_NE;
	}

	// invert conditional code
	switch (cc) {
		case CC_E: cc = CC_NE; break;
		case CC_NE: cc = CC_E; break;
		case CC_L: cc = CC_GE; break;
		case CC_LE: cc = CC_G; break;
		case CC_G: cc = CC_LE; break;
		case CC_GE: cc = CC_L; break;
	}

	// at this point, cc should be well-defined; also take note of inverted
	// order of true/false branches
	link_bbs(bb, cc, bb_false, bb_true);

	// TODO: remove
//	switch(expr->generic.type) {
//	case NT_BINOP:;
//		struct addr *addr1 = gen_rvalue(expr->binop.left, NULL, bb);
//		struct addr *addr2 = gen_rvalue(expr->binop.right, NULL, bb);
//		quad_new(bb, OC_CMP, NULL, addr1, addr2);
//
//		// struct addr *addr_bt = gen
//		switch(expr->binop.op) {
//		case '<':	link_basic_block(bb, BR_LT, bb_true, bb_false);	break;
//		case '>':	link_basic_block(bb, BR_GT, bb_true, bb_false);	break;
//		case LTEQ:	link_basic_block(bb, BR_LTEQ, bb_true, bb_false);	break;
//		case GTEQ:	link_basic_block(bb, BR_GTEQ, bb_true, bb_false);	break;
//		case EQEQ:	link_basic_block(bb, BR_EQ, bb_true, bb_false);	break;
//		case NOTEQ:	link_basic_block(bb, BR_NEQ, bb_true, bb_false);	break;
//		default:	yyerror("binop in conditional expr\n");
//		}
//		break;
//	}
}

void generate_cont_break_quads(union astnode *stmt, struct basic_block *bb)
{
	NYI("continue/break quad generation");

//	if (NT(stmt)) {
//		link_basic_block(bb, ALWAYS, cur_loop->bb_break, NULL);
//	} else {
//		if(NT(stmt)) {
//			link_basic_block(bb, ALWAYS, cur_loop->bb_cont, NULL);
//		}
//	}
}

void link_bbs(struct basic_block *bb, enum cc cc,
	struct basic_block *bb_def, struct basic_block *bb_cond)
{
	bb->branch_cc = cc;

	bb->next_def = bb_def;
	bb->next_cond = bb_cond;
}

// TODO: remove
//struct basic_block *link_basic_block(struct basic_block *bb,
//	enum branches branch, struct basic_block *bb_true,
//	struct basic_block *bb_false)
//{
//	bb->branch = branch;
//
//	bb->prev = prev;
//	bb->next_cond = next;
//
//	return bb;
//}