#include <quads/exprquads.h>
#include <quads/sizeof.h>
#include <parser.tab.h>

/**
 * helper function to demote astnode array to pointer
 *
 * @param decl 		astnode representation of pointer type
 * @return 		pointer type if input type is array, else original type
 */
static union astnode *astnode_demote_array(union astnode *decl)
{
	union astnode *ptr;

	// if not array type, ignore
	if (NT(decl) != NT_DECLARATOR_ARRAY) {
		return decl;
	}

	ALLOC_TYPE(ptr, NT_DECLARATOR_POINTER);
	ptr->decl_pointer.of = decl->decl_array.of;

	return ptr;

	// memory management is horrible -- this will cause really annoying
	// dangling pointers if you actually try to free memory
}

/**
 * helper function to demote array to pointer type if not direct arg to sizeof;
 * if not an array type, no-op
 *
 * @param addr		struct addr to demote (if array type)
 */
static void demote_array(struct addr *addr)
{
	union astnode *ptr;

	if ((ptr = astnode_demote_array(addr->decl)) != addr->decl) {
		addr->decl = ptr;
		addr->size = astnode_sizeof_type(ptr);
	}
}

/**
 * helper function to wrap a declaration in a pointer-to-(that type) typespec
 *
 * for use with the addressof operator
 *
 * @param decl		astnode representation of type to wrap
 * @return		pointer to input type
 */
static union astnode *create_pointer_to(union astnode *decl)
{
	union astnode *ptr;

	ALLOC_TYPE(ptr, NT_DECLARATOR_POINTER);
	ptr->decl_pointer.of = decl;
	return ptr;
}

/**
 * helper function to generate a typespec emulating size_t (which acts like an
 * unsigned long long)
 *
 * For use when generating a compile-time constant representing some pointer
 * constant (i.e., for use with sizeof and pointer arithmetic)
 *
 * @return		unsigned long long astnode typespec representation
 */
static union astnode *create_size_t(void)
{
	union astnode *ts;

	ALLOC_TYPE(ts, NT_TS_SCALAR);
	ts->ts_scalar.basetype = BT_INT;
	ts->ts_scalar.modifiers.lls = LLS_LONG_LONG;
	ts->ts_scalar.modifiers.sign = SIGN_UNSIGNED;

	return ts;
}

struct addr *gen_rvalue(union astnode *expr, struct addr *dest,
	struct basic_block *bb)
{
	struct addr *src1, *src2, *tmp, *tmp2;
	struct quad *quad;
	enum addr_mode mode;
	union astnode *ts;
	enum opcode op;
	unsigned subtype_size;

	// null expression
	// this shouldn't happen but this is here as a safety measure
	if (!expr) {
		yyerror("quadgen: empty expression in gen_rvalue()");
		return NULL;
	}

	// TODO: implement remaining operators
	switch (NT(expr)) {

	// symbol
	case NT_DECL:
		// abstract typename (e.g., cast), not an lvalue
		if (!expr->decl.ident) {
			// cannot be assigned to a value, so dest should be NULL
			if (dest) {
				// shouldn't happen in the grammar, but just
				// to be safe
				yyerror_fatal("quadgen: attempting to assign"
					" abstract type to lvalue");
				return NULL;
			}

			dest = addr_new(AT_AST, expr->decl.components);
			dest->val.astnode = expr;
			return dest;
		}

		return gen_lvalue(expr, bb, NULL, dest, 0);

	// constant number
	case NT_NUMBER:
		// don't support expressions with non-integral types
		if (expr->num.ts->ts_scalar.basetype != BT_INT
			&& expr->num.ts->ts_scalar.basetype != BT_CHAR) {
			yyerror_fatal("only int, char types allowed in"
				      " expressions at this time");
		}

		// convert into const
		src1 = addr_new(AT_CONST, expr->num.ts);
		*((uint64_t *)src1->val.constval) =
			*((uint64_t *)expr->num.buf);

		if (dest) {
			quad_new(bb, OC_MOV, dest, src1, NULL);
		} else {
			dest = src1;
		}
		return dest;

		// unary operator
	case NT_UNOP:

		// special cases: sizeof; these do not generate quads and
		// do not demote arrays
		switch (expr->unop.op) {

		// sizeof with a symbol or constexpr
		case SIZEOF:
#if DEBUG2
			// generate quads for expression in sizeof operand
			// for debugging purposes only
			src1 = gen_rvalue(expr->unop.arg, NULL, bb);
#else
			// generate quads to a dummy basic block
			src1 = gen_rvalue(expr->unop.arg, NULL,
				dummy_basic_block_new());
#endif

			// src2 is the resultant struct addr (static number
			// after compilation)
			src2 = addr_new(AT_CONST, create_size_t());
			*((uint64_t *) src2->val.constval)
				= src1->type == AT_CONST
				  ? src1->size
				  : astnode_sizeof_type(src1->decl);

			if (dest) {
				quad_new(bb, OC_MOV, dest, src2, NULL);
			} else {
				dest = src2;
			}
			return dest;

		// sizeof with a typename
		case 's':
			src1 = addr_new(AT_CONST, create_size_t());
			*((uint64_t *) src1->val.constval)
				= astnode_sizeof_type(expr->unop.arg
							      ->decl.components);

			if (dest) {
				quad_new(bb, OC_MOV, dest, src1, NULL);
			} else {
				dest = src1;
			}
			return dest;
		}

		// TODO: implement these
		// lvalue unops: & (addressof), ++, -- (post inc/dec)
		switch (expr->unop.op) {
		// pointer deref
		case '*':
			// lvalue is the deref expression itself
			return gen_lvalue(expr, bb, NULL, dest, 0);

		// addressof
		case '&':
			// lvalue is the argument to the addressof expression
			return gen_lvalue(expr->unop.arg, bb, NULL, dest, 1);

		// postincrement/decrement operators
		case PLUSPLUS:
			NYI("postinc");
			return NULL;

		case MINUSMINUS:
			NYI("preinc");
			return NULL;
		}

		// other rvalue unops: these generate quads and demote arrays
		src1 = gen_rvalue(expr->unop.arg, NULL, bb);
		demote_array(src1);

		switch (expr->unop.op) {

		// TODO: implement addressof operator

		// TODO: implement fncall (is it a unop?)

		}
		break;

	// binary operator
	case NT_BINOP:
		// special binop: assignment
		if (expr->binop.op == '=') {
			return gen_assign(expr, dest, bb);
		}

		src1 = gen_rvalue(expr->binop.left, NULL, bb);
		src2 = gen_rvalue(expr->binop.right, NULL, bb);

		// after here, no chance of an array that doesn't get demoted
		// to a pointer (that only happens for sizeof)
		demote_array(src1);
		demote_array(src2);

		switch (expr->binop.op) {
		// helper for pointer arithmetic: is array/pointer
		#define AOP(addr) (NT(addr->decl) == NT_DECLARATOR_POINTER)

		// TODO: for pointer arithmetic make sure operands are
		// 	the same type
		// TODO: for pointer arithmetic make sure other operand is an
		// 	integral type
		// addition (regular and pointer)
		case '+':
			// make the first one the pointer, if applicable
			if (AOP(src2)) {
				tmp = src2;
				src2 = src1;
				src1 = tmp;
			}

			// error case: pointer + pointer
			if (AOP(src1) && AOP(src2)) {
				yyerror_fatal("pointer + pointer");
				return NULL;
			}

			// special case: pointer + int;
			if (AOP(src1)) {
				// p+i => p + sizeof(*p)*i
				// replace src2
				tmp = addr_new(AT_CONST, create_size_t());
				*((uint64_t *)tmp->val.constval) =
					astnode_sizeof_type(src1->decl->
						decl_pointer.of);

				tmp2 = tmp_addr_new(create_size_t());
				quad_new(bb, OC_MUL, tmp2, tmp, src2);
				src2 = tmp2;
			}

			// create new tmp
			if (!dest) {
				dest = tmp_addr_new(src1->decl);
			}
			quad_new(bb, OC_ADD, dest, src1, src2);
			return dest;

		// subtraction (regular and pointer)
		case '-':
			// integer - pointer invalid
			if (!AOP(src1) && AOP(src2)) {
				yyerror_fatal("integer - pointer");
				return NULL;
			}

			// pointer - integer
			else if (AOP(src1) && !AOP(src2)) {
				// p-i = p - sizeof(*p) * i
				// need to modify src2
				tmp = addr_new(AT_CONST, create_size_t());
				*((uint64_t *)tmp->val.constval) =
					astnode_sizeof_type(src1->decl->
						decl_pointer.of);

				tmp2 = tmp_addr_new(create_size_t());
				quad_new(bb, OC_MUL, tmp2, tmp, src2);
				src2 = tmp2;
			}

			if (!dest) {
				dest = tmp_addr_new(src1->decl);
			}
			quad = quad_new(bb, OC_SUB, dest, src1, src2);

			// pointer - pointer
			if (AOP(src1) && AOP(src2)) {
				// p1-p2 = p1-p2 / sizeof(*p1)
				// inject a division operation afterward
				tmp = tmp_addr_new(create_size_t());
				*((uint64_t *)tmp->val.constval) =
					astnode_sizeof_type(src1->decl->
						decl_pointer.of);

				tmp2 = tmp_addr_new(create_size_t());
				quad->dest = tmp2;

				quad_new(bb, OC_DIV, dest, tmp2, tmp);
			}

			return dest;

		case '*':
			if (!dest) {
				dest = tmp_addr_new(src1->decl);
			}
			quad_new(bb, OC_MUL, dest, src1, src2);
			return dest;

		case '/':
			if (!dest) {
				dest = tmp_addr_new(src1->decl);
			}
			quad_new(bb, OC_MUL, dest, src1, src2);
			return dest;

		// explicit type cast
		case 'c':
			// TODO: should check if dest type is compatible
			// 	not going to do that now out of time

			if (!dest) {
				dest = tmp_addr_new(src1->decl);
			}
			quad_new(bb, OC_CAST, dest, src2, NULL);
			return dest;
		}
		break;

	default:
		NYI("quadgen: other expression type quad generation");
	}

	yyerror_fatal("quadgen: invalid fallthrough; some expr not handled");
	return NULL;
}

struct addr *gen_lvalue(union astnode *expr, struct basic_block *bb,
	enum addr_mode *mode, struct addr *dest, int addrof)
{
	struct addr *tmp;
	union astnode *ts;

	switch (NT(expr)) {

	// variable
	case NT_DECL:

		// if abstract, not an lvalue; fallthrough to error
		if (!expr->decl.ident) {
			break;
		}

		switch (NT(expr->decl.components)) {

		case NT_DECLSPEC:
			if (NT(expr->decl.components->declspec.ts) !=
				NT_TS_SCALAR) {
				yyerror_fatal("struct/union lvalue"
					      " not supported (yet)");
				return NULL;
			}

			// TODO: don't allow non-int data types

			// note: no break; other variables fallthrough

		// pointer can be used as an lvalue but not array/function
		case NT_DECLARATOR_POINTER:
			if (mode) {
				*mode = AM_DIRECT;
			}

			tmp = addr_new(AT_AST, expr->decl.components);
			tmp->val.astnode = expr;

			// regular
			if (!addrof) {
				if (!dest) {
					dest = tmp;
				} else {
					quad_new(bb, OC_MOV, dest, tmp, NULL);
				}
			}
			// addressof
			else {
				if (!dest) {
					dest = tmp_addr_new(create_pointer_to(
						tmp->decl));
				}
				quad_new(bb, OC_LEA, dest, tmp, NULL);
			}

			return dest;

		// treat array as pointer
		case NT_DECLARATOR_ARRAY:
			if (mode) {
				*mode = AM_DIRECT;
			}

			tmp = addr_new(AT_AST, expr->decl.components);
			tmp->val.astnode = expr;

			// regular
			if (!addrof) {
				if (!dest) {
					dest = tmp_addr_new(tmp->decl);
				}
				quad_new(bb, OC_LEA, dest, tmp, NULL);
			}
			// addressof
			else {
				if (!dest) {
					dest = tmp_addr_new(create_pointer_to(
						tmp->decl));
				}
				// noop/reinterpret cast
				quad_new(bb, OC_CAST, dest, tmp, NULL);
			}

			return dest;
		}

		// fallthrough: invalid lvalue (shouldn't happen)
		break;

	// deref
	case NT_UNOP:
		// only valid operation that returns an lvalue is deref,
		// if not fallthrough to error
		if (expr->unop.op != '*') {
			break;
		}

		if (mode) {
			*mode = AM_INDIRECT;
		}

		// if addrof, we will be eliding this, so output to dest;
		// but if not, we need an intermediate value
		tmp = gen_rvalue(expr->unop.arg, addrof?dest:NULL, bb);
		demote_array(tmp);

		// check that the rvalue is a pointer type
		if (NT(tmp->decl) != NT_DECLARATOR_POINTER) {
			yyerror_fatal("dereferencing non-pointer type");
		}

		// get the type that the pointer is pointing to
		ts = tmp->decl->decl_pointer.of;

		// regular
		if (!addrof) {
			// if being used as the LHS of an assignment
			if (mode) {
				if (NT(ts) == NT_DECLARATOR_ARRAY
					|| NT(ts) == NT_DECLARATOR_FUNCTION) {
					// fallthrough
					break;
				}

				dest = tmp;
			}
			// if being used as an rvalue
			else {
				// create addr of underlying type for result
				if (!dest) {
					dest = tmp_addr_new(ts);
				}

				// not pointing to an array
				if (NT(ts) != NT_DECLARATOR_ARRAY) {
					quad_new(bb, OC_LOAD, dest, tmp, NULL);
				}
				// pointing to an array (no-op/reinterpret cast)
				else {
					quad_new(bb, OC_CAST, dest, tmp, NULL);
				}
			}
		}
		// addressof (elide LOAD quad)
		else {
			if (!dest) {
				dest = tmp;
			}
		}

		return dest;
	}

	yyerror_fatal("quadgen: gen_lvalue fallthrough: invalid lvalue");
	return NULL;
}

struct addr *gen_assign(union astnode *expr, struct addr *target,
	struct basic_block *bb)
{
	struct addr *dest, *src;
	enum addr_mode mode;

	if (NT(expr) != NT_BINOP || expr->binop.op != '=') {
		yyerror_fatal("quadgen: gen_assign(): not an assignment");
		return NULL;
	}

	// generate lvalue; if invalid lvalue, will report and panic in
	// gen_lvalue()
	dest = gen_lvalue(expr->binop.left, bb, &mode, NULL, 0);

	// TODO: don't allow assignment to array/function lvalue

	if (mode == AM_DIRECT) {
		src = gen_rvalue(expr->binop.right, dest, bb);
	} else {
		src = gen_rvalue(expr->binop.right, NULL, bb);
		quad_new(bb, OC_STORE, NULL, src, dest);
	}

	// directly being assigned to a memory location, issue a MOV
	if (target) {
		quad_new(bb, OC_MOV, target, src, NULL);
	}

	return src;
}
