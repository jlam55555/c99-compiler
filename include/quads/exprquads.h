/**
 * Utilities for generating quads for expressions.
 */

#ifndef EXPRQUADS_H
#define EXPRQUADS_H

#include <parser/astnode.h>
#include <quads/quads.h>

/**
 * linked list of astnode string for target code generation
 */
extern union astnode *string_ll;

/**
 * helper function to generate a typespec emulating size_t (which acts like an
 * unsigned long long)
 *
 * For use when generating a compile-time constant representing some pointer
 * constant (i.e., for use with sizeof and pointer arithmetic)
 *
 * @return		unsigned long long astnode typespec representation
 */
union astnode *create_size_t(void);

/**
 * helper function to generate a typespec emulating int (e.g., for value of
 * a relational operator)
 *
 * @return		(regular signed) int astnode typespec representation
 */
union astnode *create_int(void);

/**
 * iteratively and recursively generates a linked-list of quads for an
 * (r-value) expression; corresponds to function of same name in lecture notes
 *
 * The use of the target address is to have more optimized 3-address quads,
 * i.e., remove extra MOV quads. (These will still have to be added back in
 * when we get to assembly code generation, as x86 is a 2-address architecture.)
 * Supply the target when the overall target of the given command is
 * a known lvalue destination.
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
 * - if cc is not NULL, then indicates that this expression is intended for
 * 	the condition codes it sets
 * 	- if relational operator, do not save cmp result to a psuedo-register,
 * 		and set which condition code is set
 * 	- if non-relational operator, then do not set condition code; the result
 * 		will be compared with 0 (implicit compare equality with 0)
 * 	- this should be mutually exclusive with dest; an rvalue should not
 * 		be (directly) used for both an assignment and a condition code
 *
 * @param expr		expression object
 * @param dest		target address, or NULL if temporary expression;
 * @param cc		when result is (directly) used for condition codes;
 * 			see semantic notes
 * @return 		addr storing result of expression
 */
struct addr *gen_rvalue(union astnode *expr, struct addr *dest, enum cc *cc);

/**
 * generate a lvalue; may also be used as a rvalue, and thus a dest may be
 * specified (and will be treated in the same way as for an rvalue)
 *
 * Handles regular variables and dereferenced values. Can be used anywhere as
 * an rvalue, or for LHS of assignments, for postinc/dec (++/--), addressof (&).
 *
 * This supports taking the address of (&) its operand, because this requires
 * the elision of the LOAD quad in the case of an indirect lvalue (e.g., *a will
 * usually generate a LOAD, but &*a should simply return a).
 *
 * semantic notes:
 * - currently only scalar integral variables are supported, struct/union and
 * 	fp lvalues are not
 *
 * @param expr		expression astnode representing lvalue
 * @param mode		set addressing mode
 * @param dest		target address, or NULL if temporary expression
 * @param addrof	whether to take the address of the current expr
 * @return		generated lvalue
 */
struct addr *gen_lvalue(union astnode *expr, enum addr_mode *mode,
	struct addr *dest, int addrof);

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
 * @return		addr of RHS of expression (for use as an rvalue)
 */
struct addr *gen_assign(union astnode *expr, struct addr *target);

#endif	// EXPRQUADS_H
