#include <quads/exprquads.h>
#include <quads/sizeof.h>
#include <parser.tab.h>

/**
 * helper function to demote array to pointer type if not direct arg to sizeof;
 * if not an array type, no-op
 *
 * @param addr		struct addr to demote (if array type)
 */
static void demote_array(struct addr *addr)
{
	union astnode *ptr;

	// if not array type, ignore
	if (NT(addr->decl) != NT_DECLARATOR_ARRAY) {
		return;
	}

	ALLOC_TYPE(ptr, NT_DECLARATOR_POINTER);
	ptr->decl_pointer.of = addr->decl->decl_array.of;
	addr->decl = ptr;

	addr->size = astnode_sizeof_type(ptr);

	// memory management is horrible -- this will cause really annoying
	// dangling pointers if you actually try to free memory
}

/**
 * helper function to generate a typespec emulating size_t (which acts like an
 * unsigned long long)
 *
 * @return typespec
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
	union astnode *ts;
	enum opcode op;
	unsigned subtype_size;

	// null expression
	// this shouldn't happen but this is here as a safety measure
	if (!expr) {
		yyerror("quadgen: empty expression in gen_rvalue()");
		return NULL;
	}

	switch (NT(expr)) {

	// symbol
	case NT_DECL:
		// if abstract, don't do anything and don't return anything
		if (!expr->decl.ident) {
			return NULL;
		}

		// don't support expressions with non-integral types
		if (NT(expr->decl.components) == NT_DECLSPEC) {
			ts = expr->decl.components->declspec.ts;

			if (NT(ts) == NT_TS_SCALAR
			    && ts->ts_scalar.basetype != BT_INT
			    && ts->ts_scalar.basetype != BT_CHAR) {
				yyerror_fatal("only int, char types allowed in"
					      " expressions at this time");
			}
		}

		// treat array as pointer (special cases are treated elsewhere)
		if (NT(expr->decl.components) == NT_DECLARATOR_ARRAY) {
			src1 = addr_new(AT_AST, expr->decl.components);
			src1->val.astnode = expr;

			if (!dest) {
				dest = tmp_addr_new(src1->decl);
			}
			quad_new(bb, OC_LEA, dest, src1, NULL);
		}

			// not a pointer
		else {
			src1 = addr_new(AT_AST, expr->decl.components);
			src1->val.astnode = expr;

			if (dest) {
				quad_new(bb, OC_MOV, dest, src1, NULL);
			} else {
				dest = src1;
			}
		}

		return dest;

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
		src1 = gen_rvalue(expr->unop.arg, NULL, bb);

		// TODO: still have to implement a lot here
		switch (expr->unop.op) {

		// sizeof with a symbol or constexpr
		case SIZEOF:
			src2 = addr_new(AT_CONST, create_size_t());
			*((uint64_t *)src2->val.constval)
				= src1->type == AT_CONST
					? src1->size
					: astnode_sizeof_type(src1->decl);

			if (dest) {
				quad_new(bb, OC_MOV, dest, src2, NULL);
			} else {
				dest = src2;
			}
			return dest;

		// sizeof with a typename (addr1 should be NULL)
		case 's':
			src1 = addr_new(AT_CONST, create_size_t());
			*((uint64_t *)src1->val.constval)
				= astnode_sizeof_type(expr->unop.arg
							      ->decl.components);

			if (dest) {
				quad_new(bb, OC_MOV, dest, src1, NULL);
			} else {
				dest = src1;
			}
			return dest;

		// pointer deref
		case '*':
			demote_array(src1);

			// check that the rvalue is a pointer type
			if (NT(src1->decl) != NT_DECLARATOR_POINTER) {
				yyerror_fatal("dereferencing non-pointer type");
			}

			// create new addr of underlying type to store the
			// result in
			if (!dest) {
				// get the type that the pointer is pointing to
				ts = src1->decl->decl_pointer.of;
				dest = tmp_addr_new(ts);
			}

			// noop not pointing to an array
			if (NT(src1->decl->decl_array.of)
				!= NT_DECLARATOR_ARRAY) {
				quad_new(bb, OC_LOAD, dest, src1, NULL);
			}
			// noop cast i.e., reinterpret pointer
			// as different size but same pointer value
			else {
				quad_new(bb, OC_CAST, dest, src1, NULL);
			}
			return dest;
		}
		break;

	// binary operator
	case NT_BINOP:
		switch(expr->binop.op){
		case '+':	op = OC_ADD;	break;
		case '-':	op = OC_SUB;	break;
		case '*':	op = OC_MUL;	break;
		case '/':	op = OC_DIV;	break;
		case '%':	op = OC_MOD;	break;
		case '&':	op = OC_AND;	break;
		case '|':	op = OC_OR;	break;
		case '^':	op = OC_XOR;	break;
		case SHL:	op = OC_SHL;	break;
		case SHR:	op = OC_SHR;	break;

		// special binop: assignment
		case '=':	return gen_assign(expr, dest, bb);
		}

		src1 = gen_rvalue(expr->binop.left, NULL, bb);
		src2 = gen_rvalue(expr->binop.right, NULL, bb);

		// after here, no chance of an array that doesn't get demoted
		// to a pointer (that only happens for sizeof)
		demote_array(src1);
		demote_array(src2);

		// TODO: +/- have to correctly implement pointer arithmetic
		// 	(have to check type of their operands, which means
		// 	that we have to associate type with struct addrs)
		switch (expr->binop.op) {
		// arithmetic
		case '+':
			// helper: is array/pointer
			#define AOP(addr) \
				(NT(addr->decl) == NT_DECLARATOR_POINTER)

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
				// insert operation to multiply src2 by sizeof
				// src1 subtype

				// see notes above
				ALLOC_TYPE(ts, NT_TS_SCALAR);
				ts->ts_scalar.basetype = BT_INT;
				ts->ts_scalar.modifiers.lls = LLS_LONG_LONG;
				ts->ts_scalar.modifiers.sign = SIGN_UNSIGNED;

				// convert into const
				tmp = addr_new(AT_CONST, ts);
				*((uint64_t *)tmp->val.constval) =
					astnode_sizeof_type(src1->decl->
						decl_pointer.of);

				tmp2 = tmp_addr_new(src2->decl);
				quad_new(bb, OC_MUL, tmp2, tmp, src2);
				src2 = tmp2;
			}

			// create new tmp
			// TODO: choose larger of two sizes
			// 	(or better yet, use real types rather than just
			// 	sizes)
			if (!dest) {
				dest = tmp_addr_new(src1->decl);
			}
			quad_new(bb, OC_ADD, dest, src1, src2);
			return dest;

		case '-':
			if (!dest) {
				dest = tmp_addr_new(src1->decl);
			}
			quad_new(bb, OC_SUB, dest, src1, src2);
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

		}
		break;

	default:
		NYI("quadgen: other expression type quad generation");
	}

	yyerror_fatal("quadgen: invalid fallthrough; some expr not handled");
	return NULL;
}

struct addr *gen_lvalue(union astnode *expr, struct basic_block *bb,
	enum addr_mode *mode)
{
	struct addr *dest;

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
				yyerror_fatal("assignment to struct/union"
					      " not supported (yet)");
				return NULL;
			}
		case NT_DECLARATOR_POINTER:

			*mode = AM_DIRECT;

			dest = addr_new(AT_AST, expr->decl.components);
			dest->val.astnode = expr;
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

		*mode = AM_INDIRECT;

		dest = gen_rvalue(expr->unop.arg, NULL, bb);

		// check that the rvalue is a pointer type
		if (NT(dest->decl) != NT_DECLARATOR_POINTER
			&& NT(dest->decl) != NT_DECLARATOR_ARRAY) {

			yyerror_fatal("dereferencing non-pointer type");
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
	dest = gen_lvalue(expr->binop.left, bb, &mode);
	if (mode == AM_DIRECT) {
		src = gen_rvalue(expr->binop.right, dest, bb);
	} else {
		src = gen_rvalue(expr->binop.right, NULL, bb);
		quad_new(bb, OC_STORE, NULL, src, dest);
	}

	// directly being assigned to a memory location, issue a MOV
	if (target) {
		quad_new(bb, OC_MOV, NULL, src, target);
	}

	return src;
}
