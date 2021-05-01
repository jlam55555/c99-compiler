/**
 * quad generation utilities for control flow statements (conditional branches,
 * loops, labels)
 */

#ifndef CFQUADS_H
#define CFQUADS_H

#include <quads/quads.h>
#include <parser/astnode.h>

/**
 * storing the current loop continue/break points
 */
struct loop {
	struct basic_block *bb_cont, *bb_break;
};

/**
 * generate for loop quads
 *
 * @param stmt		astnode representation of for statement
 */
void generate_for_quads(union astnode *stmt);

/**
 * generates quads for a while loop
 *
 * semantics:
 * - After this function completes, cur_bb will be .BB.NEXT
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
 */
void generate_while_quads(union astnode *stmt);

/**
 * generate quads for if/else statements; performs condition inversion for
 * efficiency
 *
 * semantics:
 * - After this function completes, cur_bb will be .BB.NEXT
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
 */
void generate_if_else_quads(union astnode *expr);

/**
 * generate quads for conditional expression (if, for, while conditionals);
 * has the capability of performing condition inversion if desired
 *
 * @param expr		expression in if
 * @param bb_true	basic block then
 * @param bb_false	basic block false
 * @param invert	whether to invert condition
 */
void generate_conditional_quads(union astnode *expr,
	struct basic_block *bb_true, struct basic_block *bb_false, int invert);

/**
 * handling jump (continue/break) quads (not currently supporting goto)
 *
 * return statement handled separately because it generates a quad and handles
 * a value
 *
 * this calls link_bb() and thus end the block; doesn't actually generate
 * any quads
 *
 * @param stmt		astnode continue/break stmt
 */
void gen_jmp_quads(union astnode *stmt);

/**
 * handling return quads
 * @param stmt
*/
void gen_ret_quads(union astnode *stmt);


/**
 * links the current basic block to its successor(s), and reverses the quads
 * in the current bb
 *
 * semantics:
 * - since it only makes sense to link the current bb to the next when all the
 * 	quads of the current basic block are finished, this acts as a de facto
 * 	indication that the current basic block is complete; thus it performs
 * 	the quad reversal as well
 *
 * @param cc		condition code to branch on, or CC_UNSPEC for
 * 			unconditional branch
 * @param bb_def	default next branch
 * @param bb_cond	conditional next branch (NULL for unconditional branch)
 */
void link_bb(enum cc cc, struct basic_block *bb_def,
	struct basic_block *bb_cond);

/**
 * Generates do-while statement. Almost the same as a while loop but the initial
 * jump is simply a fallthrough to the body (rather than a jump to the cond)
 *
 * @param stmt		astnode representation of do-while statement
 */
void generate_do_while_quads(union astnode *stmt);

#endif // CFQUADS_H