/**
 * quad generation utilities for control flow statements (conditional branches,
 * loops, labels)
 */

#ifndef CFQUADS_H
#define CFQUADS_H

#include <quads/quads.h>
#include <parser/astnode.h>

// TODO: remove; is replaced with condition codes, which are not specific
// 	to branching
//enum branches {
//	NEVER=0,
//	ALWAYS=1,
//	BR_LT,
//	BR_GT,
//	BR_EQ,
//	BR_NEQ,
//	BR_LTEQ,
//	BR_GTEQ,
//};

/**
 * TODO: need documentation
 */
// TODO: move this to quads.h?
struct loop {
	struct basic_block *bb_cont, *bb_break;
	struct loop *prev;
};

/**
 * TODO: need documentation
 */
void generate_for_quads(union astnode *stmt, struct basic_block *bb);

/**
 * generates quads for a while loop
 *
 * semantics:
 * - This uses the second form from Hak's notes, which is usually more than
 * 	the more straightforward form. Note that this doesn't use condition
 * 	inversion. It looks like:
 *
 * .BB.INITIALJUMP:
 * 	JMP .BB.CONDITION
 * 	; fallthrough to .BB.LOOPBODY
 * .BB.LOOPBODY:
 * 	; loop body statements
 * 	; fallthrough to .BB.CONDITION
 * .BB.CONDITION:
 * 	; perform condition here (no inversion)
 * 	JMPcc .BB.LOOPBODY
 * 	; fallthrough to .BB.NEXT
 * .BB.NEXT:
 * 	; successor of while loop
 * 	; ...
 *
 * @param stmt		astnode representation of while statement
 * @param bb		current basic block
 * @return		basic block that succeeds while loop (.BB.NEXT)
 */
struct basic_block *generate_while_quads(union astnode *stmt,
	struct basic_block *bb);

/**
 * generate quads for if/else statements; performs condition inversion for
 * efficiency
 *
 * semantics:
 * - for a basic if stmt without else, the result looks like:
 *
 * .BB.COND:
 * 	; perform (inverted) condition here
 * 	JMPcc .BB.TRUE:
 * 	; fallthrough to .BB.NEXT
 * .BB.NEXT:
 * 	; successor of if statement
 * 	; ...
 * .BB.TRUE:
 * 	; statements in true branch
 * 	JMP .BB.NEXT
 *
 * - for an if stmt with else, the result looks like:
 *
 * .BB.COND:
 * 	; perform (inverted) condition here
 * 	JMPcc .BB.TRUE:
 * 	; fallthrough to .BB.FALSE
 * .BB.FALSE:
 * 	; statements in else branch
 * 	; fallthrough to .BB.NEXT
 * .BB.NEXT:
 * 	; successor of if/else stmt
 * 	; ...
 * .BB.TRUE:
 * 	; statements in true branch
 * 	JMP .BB.NEXT
 *
 * @param expr		astnode representation if statement
 * @param bb		current basic block
 * @return		basic block that succeeds if stmt
 */
struct basic_block *generate_if_else_quads(union astnode *expr,
	struct basic_block *bb);

/**
 * generate quads for conditional expression (if, for, while conditionals);
 * has the capability of performing condition inversion if desired
 *
 * @param expr		expression in if
 * @param bb		current basic block
 * @param bb_true	basic block then
 * @param bb_false	basic block false
 * @param invert	whether to invert condition
 */
void generate_conditional_quads(union astnode *expr, struct basic_block *bb,
	struct basic_block *bb_true, struct basic_block *bb_false, int invert);

/**
 * link basic blocks, generating the appropriate JMPcc command
 *
 * @param bb 		current basic block
 * @param cc		condition code to branch on, or CC_UNSPEC for
 * 			unconditional branch
 * @param bb_def	default next branch
 * @param bb_cond	conditional next branch (NULL for unconditional branch)
 */
void link_bbs(struct basic_block *bb, enum cc cc,
	struct basic_block *bb_def, struct basic_block *bb_cond);

// TODO: remove
//struct basic_block *link_basic_block(struct basic_block *bb,
//	enum branches branch, struct basic_block *prev,
//	struct basic_block *next);

/**
 * TODO: need documentation
 */
void generate_do_while_quads(union astnode *stmt, struct basic_block *bb);

#endif // CFQUADS_H