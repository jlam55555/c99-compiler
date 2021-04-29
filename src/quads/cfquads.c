#include <quads/cfquads.h>
#include <quads/quads.h>
#include <quads/exprquads.h>
#include <malloc.h>

// TODO: logical operators

// used to hold state information about the current loops; information for
// parent loops is stored on the stack frame
static struct loop *cur_loop;

struct loop *loop_new(void)
{
	struct loop *lp = malloc(sizeof(struct loop));
	lp->prev = cur_loop;
	return lp;
}

void generate_for_quads(union astnode *stmt)
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
void generate_do_while_quads(union astnode *stmt)
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
//	gen_stmt_quads(stmt->stmt_while.body, bb);
//
//	link_basic_block(bb, ALWAYS, bb_cond, NULL);
//	bb = bb_cond;
//
//	generate_conditional_quads(stmt->stmt_while.cond, bb, bb_body, bb_next);
//
//	bb = bb_next;
//	cur_loop = cur_loop->prev;
}

void generate_while_quads(union astnode *stmt)
{
	NYI("while quad generation");

	struct basic_block *bb_initjmp, *bb_body, *bb_cond, *bb_next;

	bb_initjmp = basic_block_new();
	bb_body = basic_block_new();
	bb_cond = basic_block_new();
	bb_next = basic_block_new();

	// first bb is simply a JMP to the condition
	cur_bb = bb_initjmp;
	link_bb(CC_ALWAYS, bb_cond, NULL);

	// generate body quads like normal
	cur_bb = bb_body;
	gen_stmt_quads(stmt->stmt_while.body);
	link_bb(CC_ALWAYS, bb_cond, NULL);

	// generate condition quads
	cur_bb = bb_cond;
	generate_conditional_quads(stmt->stmt_while.cond, bb_body, bb_next, 0);

	cur_bb = bb_next;

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
//	gen_stmt_quads(stmt->stmt_while.body, bb);
//
//	link_basic_block(bb, ALWAYS, bb_cond, NULL);
//
//	bb = bb_next;
//	cur_loop = cur_loop->prev;
}

void generate_if_else_quads(union astnode *expr)
{
	struct basic_block *bb_true = basic_block_new();
	struct basic_block *bb_false = basic_block_new();
	struct basic_block *bb_next;

	if (expr->stmt_if_else.elsestmt) {
		bb_next = basic_block_new();
	} else {
		bb_next = bb_false;
	}

	generate_conditional_quads(expr->stmt_if_else.cond,
		bb_true, bb_false, 1);

	// generate statements for true branch
	cur_bb = bb_true;
	gen_stmt_quads(expr->stmt_if_else.ifstmt);
	link_bb(CC_ALWAYS, bb_next, NULL);

	// generate statements for else branch
	if (expr->stmt_if_else.elsestmt) {
		cur_bb = bb_false;
		gen_stmt_quads(expr->stmt_if_else.elsestmt);
		link_bb(CC_ALWAYS, bb_next, NULL);
	}

	cur_bb = bb_next;
}

void generate_conditional_quads(union astnode *expr,
	struct basic_block *bb_true, struct basic_block *bb_false, int invert)
{
	enum cc cc = CC_UNSPEC;
	struct addr *cond, *zero;

	cond = gen_rvalue(expr, NULL, &cc);

	// if cc is not set, then create compare and set cc to CC_NE (nonzero,
	// default truthy condition)
	// note: the TEST opcode is more efficient than the CMP opcode, but we
	// use CMP for sake of simplicity
	if (cc == CC_UNSPEC) {
		zero = addr_new(AT_CONST, create_size_t());
		*((uint64_t*)zero->val.constval) = 0;

		quad_new(OC_CMP, NULL, cond, zero);
		cc = CC_NE;
	}

	// at this point, cc should be well-defined
	if (invert) {
		// invert conditional code
		switch (cc) {
			case CC_E: cc = CC_NE; break;
			case CC_NE: cc = CC_E; break;
			case CC_L: cc = CC_GE; break;
			case CC_LE: cc = CC_G; break;
			case CC_G: cc = CC_LE; break;
			case CC_GE: cc = CC_L; break;
		}

		link_bb(cc, bb_false, bb_true);
	} else {
		link_bb(cc, bb_true, bb_false);
	}

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

/**
 * reverse quads in cur_bb when it is complete (i.e., in link_bb)
 *
 * uses same O(N) algorithm as in decl_reverse (decl.c); see that function
 * for more details
 */
static void reverse_quads(void)
{
	struct quad *a, *b, *c;

	if (!(a = cur_bb->ll) || !(b = _LL_NEXT(a, next))) {
		return;
	}

	c = _LL_NEXT(b, next);
	_LL_NEXT(a, next) = NULL;

	while (c) {
		_LL_NEXT(b, next) = a;
		a = b;
		b = c;
		c = _LL_NEXT(c, next);
	}

	_LL_NEXT(b, next) = a;
	cur_bb->ll = b;
}

void link_bb(enum cc cc, struct basic_block *bb_def,
	struct basic_block *bb_cond)
{
	cur_bb->branch_cc = cc;

	cur_bb->next_def = bb_def;
	cur_bb->next_cond = bb_cond;

	// reverse all quads in cur_bb; see function comment for an explanation
	reverse_quads();
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