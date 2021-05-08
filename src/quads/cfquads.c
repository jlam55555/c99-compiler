#include <quads/cfquads.h>
#include <quads/quads.h>
#include <quads/exprquads.h>
#include <malloc.h>

// used to hold state information about the current loops; information for
// parent loops is stored on the stack frame
static struct loop *cur_loop;

void generate_for_quads(union astnode *stmt)
{
	struct loop *prev_loop, loop;
	struct basic_block *bb_cond = basic_block_new(0);
	struct basic_block *bb_body = basic_block_new(0);
	struct basic_block *bb_update = basic_block_new(0);
	struct basic_block *bb_next = basic_block_new(0);

	prev_loop = cur_loop;
	cur_loop = &loop;
	cur_loop->bb_cont = bb_update;
	cur_loop->bb_break = bb_next;

	// initial statment; always jumps to condition
	if (stmt->stmt_for.init) {
		gen_assign(stmt->stmt_for.init, NULL);
	}
	link_bb(CC_ALWAYS, bb_cond, NULL);

	// gen body
	cur_bb = bb_body;
	bb_ll_push(cur_bb);
	gen_stmt_quads(stmt->stmt_for.body);
	link_bb(CC_ALWAYS, bb_update, NULL);

	// link to update
	cur_bb = bb_update;
	bb_ll_push(cur_bb);
	gen_rvalue(stmt->stmt_for.update, NULL, NULL);
	link_bb(CC_ALWAYS, bb_cond, NULL);

	// condition body
	cur_bb = bb_cond;
	bb_ll_push(cur_bb);
	if (stmt->stmt_for.cond) {
		generate_conditional_quads(stmt->stmt_for.cond, bb_next,
			bb_body, 0);
	} else {
		link_bb(CC_ALWAYS, bb_body, NULL);
	}

	// restore prev loop
	cur_loop = prev_loop;

	cur_bb = bb_next;
	bb_ll_push(cur_bb);

}

// TODO: can probably merge this with while loops, only one opcode difference
void generate_do_while_quads(union astnode *stmt)
{
	struct basic_block *bb_initjmp, *bb_body, *bb_cond, *bb_next;
	struct loop *prev_loop, loop;

	bb_body = basic_block_new(0);
	bb_cond = basic_block_new(0);
	bb_next = basic_block_new(0);

	// save previous loop and create new one
	prev_loop = cur_loop;
	cur_loop = &loop;
	cur_loop->bb_cont = bb_cond;
	cur_loop->bb_break = bb_next;

	// in current bb set JMP to body
	link_bb(CC_ALWAYS, bb_body, NULL);

	// Generate body quad
	cur_bb = bb_body;
	bb_ll_push(cur_bb);
	gen_stmt_quads(stmt->stmt_do_while.body);

	//link to condition
	link_bb(CC_ALWAYS, bb_cond, NULL);
	cur_bb = bb_cond;
	bb_ll_push(cur_bb);
	generate_conditional_quads(stmt->stmt_do_while.cond,
		bb_body, bb_next, 0);

	// restore prev loop
	cur_loop = prev_loop;

	cur_bb = bb_next;
	bb_ll_push(cur_bb);


}

void generate_while_quads(union astnode *stmt)
{
	struct basic_block *bb_initjmp, *bb_body, *bb_cond, *bb_next;
	struct loop *prev_loop, loop;

	bb_body = basic_block_new(0);
	bb_cond = basic_block_new(0);
	bb_next = basic_block_new(0);

	// save previous loop and create new one
	prev_loop = cur_loop;
	cur_loop = &loop;
	cur_loop->bb_break = bb_next;
	cur_loop->bb_cont = bb_cond;

	// in current bb, simply JMP to the condition
	link_bb(CC_ALWAYS, bb_cond, NULL);

	// generate body quads like normal
	cur_bb = bb_body;
	bb_ll_push(cur_bb);
	gen_stmt_quads(stmt->stmt_while.body);
	link_bb(CC_ALWAYS, bb_cond, NULL);

	// generate condition quads
	cur_bb = bb_cond;
	bb_ll_push(cur_bb);
	generate_conditional_quads(stmt->stmt_while.cond, bb_body, bb_next, 0);

	// restore prev loop
	cur_loop = prev_loop;

	cur_bb = bb_next;
	bb_ll_push(cur_bb);


}

void generate_if_else_quads(union astnode *expr)
{
	struct basic_block *bb_true, *bb_false, *bb_next;

	bb_true = basic_block_new(0);
	bb_false = basic_block_new(0);

	if (expr->stmt_if_else.elsestmt) {
		bb_next = basic_block_new(0);
	} else {
		bb_next = bb_false;
	}

	generate_conditional_quads(expr->stmt_if_else.cond,
		bb_true, bb_false, 1);

	// generate statements for true branch
	cur_bb = bb_true;
	bb_ll_push(cur_bb);
	gen_stmt_quads(expr->stmt_if_else.ifstmt);
	link_bb(CC_ALWAYS, bb_next, NULL);

	// generate statements for else branch
	if (expr->stmt_if_else.elsestmt) {
		cur_bb = bb_false;
		bb_ll_push(cur_bb);
		gen_stmt_quads(expr->stmt_if_else.elsestmt);
		link_bb(CC_ALWAYS, bb_next, NULL);
	}

	cur_bb = bb_next;
	bb_ll_push(cur_bb);
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

		link_bb(cc, bb_true, bb_false);
	} else {
		link_bb(cc, bb_false, bb_true);
	}


}

void gen_jmp_quads(union astnode *stmt)
{
	// non-fatal error if break/continue not within loop
	if (!cur_loop) {
		yyerror("break/continue not within loop; ignoring");
		return;
	}

	switch (NT(stmt)) {
	case NT_STMT_BREAK:
		link_bb(CC_ALWAYS, cur_loop->bb_break, NULL);
		return;
	case NT_STMT_CONT:
		link_bb(CC_ALWAYS, cur_loop->bb_cont, NULL);
		return;
	}

	// shouldn't reach this point
	yyerror_fatal("quadgen: invalid jmp (continue/break) statement");


}

void gen_ret_quads(union astnode *stmt){
	struct addr *addr_ret = gen_rvalue(stmt->stmt_return.rt, NULL, NULL);
	quad_new(OC_RET, NULL, addr_ret, NULL);
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
	// may already be finalized if break/continue/return already called
	// in this block
	if (cur_bb->finalized) {
		return;
	}

	cur_bb->branch_cc = cc;

	cur_bb->next_def = bb_def;
	cur_bb->next_cond = bb_cond;

	// reverse all quads in cur_bb; see function comment for an
	// explanation
	reverse_quads();

	// set basic block finished -- anything after this will throw
	// an error;
	cur_bb->finalized = 1;
}