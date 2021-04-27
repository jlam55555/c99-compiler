/**
 * quad generation utilities for control flow statements (conditional branches,
 * loops, labels)
 */

#ifndef CFQUADS_H
#define CFQUADS_H

#include <parser/astnode.h>

/**
 * TODO: need documentation
 */
enum branches {
	NEVER=0,
	ALWAYS=1,
	BR_LT,
	BR_GT,
	BR_EQ,
	BR_NEQ,
	BR_LTEQ,
	BR_GTEQ,
};

/**
 * TODO: need documentation
 */
struct loop {
	struct basic_block *bb_cont, *bb_break;
	struct loop *prev;
};

/**
 * TODO: need documentation
 */
void generate_for_quads(union astnode *stmt, struct basic_block *bb);

/**
 * generate quads for if else statements
 *
 * @param expr		expression in if
 * @param bb		current basic block
 */
void generate_if_else_quads(union astnode *expr, struct basic_block *bb);

/**
 * generate quads for conditional expression
 *
 * @param expr		expression in if
 * @param bb		current basic block
 * @param Bt		basic block Then
 * @param Bf		basic block False
 */
void generate_conditional_quads(union astnode *expr, struct basic_block *bb,
	struct basic_block *Bt, struct basic_block *Bf);

/**
 * link basic blocks
 * @param bb 		current
 * @param branch	branching
 * @param prev		previous bb
 * @param next		next bb
 */
struct basic_block *link_basic_block(struct basic_block *bb,
	enum branches branch, struct basic_block *prev,
	struct basic_block *next);

/**
 * TODO: need documentation
 */
void generate_while_quads(union astnode *stmt, struct basic_block *bb);
void generate_do_while_quads(union astnode *stmt, struct basic_block *bb);

#endif // CFQUADS_H