/**
 *
 */

#ifndef EXPRQUADS_H
#define EXPRQUADS_H

#include <parser/astnode.h>
#include <quads/quads.h>

/**
 * iteratively and recursively generates a linked-list of quads for an
 * (r-value) expression; corresponds to function of same name in lecture notes
 *
 * The use of the target address is to have more optimized 3-address quads,
 * i.e., remove extra MOV quads. (These will still have to be added back in
 * when we get to assembly code generation, as x86 is a 2-address architecture.)
 *
 * semantic notes:
 * - currently no support for floating point in expressions, only simple
 * 	scalar types (char, ([unspec]|short|long|long long) int)
 * - currently no support for signed/unsigned type conversions
 * - if primary expression (scalar or constant):
 * 	- if no target, return as a struct addr
 * 	- if target is specified, emit MOV opcode
 * - if assignment (=): see gen_assign()
 * - if unary/binary expression, emit quad
 * 	- if dest is NULL, a temporary is created
 *	- if target, it is the destination of the quad
 *
 * @param expr		expression object
 * @param dest		target address, or NULL if temporary expression
 * @param bb		basic block to add quads to
 * @return 		addr storing result of expression
 */
struct addr *gen_rvalue(union astnode *expr, struct addr *dest,
	struct basic_block *bb);

/**
 * generate a lvalue (in the case of variable assignment)
 *
 * semantic notes:
 * - only is successful if expr is a variable (memory address) or deref
 * - currently only scalar variables are supported, struct/union assignments
 *  	are not
 *
 * @param expr		expression astnode representing lvalue
 * @param bb		containing basic block
 * @param mode		set addressing mode
 * @return		generated lvalue
 */
struct addr *gen_lvalue(union astnode *expr, struct basic_block *bb,
	enum addr_mode *mode);

/**
 * handle assignment expressions
 *
 * semantic notes:
 * - returns value in case it is used as an rvalue (note: assignment is an
 * 	rvalue in C (section 6.5.16) but an lvalue in C++ (section 5.17))
 * - if multiple chained assignments (i.e., dest is specified), then will also
 * 	emit a MOV instruction
 *
 * @param expr		assignment astnode expression
 * @param target	destination if directly chained assignment to a variable
 * @param bb		containing basic block
 * @return		addr of RHS of expression (for use as an rvalue)
 */
struct addr *gen_assign(union astnode *expr, struct addr *target,
	struct basic_block *bb);

#endif	// EXPRQUADS_H
