#include <quads/exprquads.h>
#include <quads/sizeof.h>
#include <quads/cfquads.h>
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

union astnode *create_size_t(void)
{
	union astnode *ts;

	ALLOC_TYPE(ts, NT_TS_SCALAR);
	ts->ts_scalar.basetype = BT_INT;
	ts->ts_scalar.modifiers.lls = LLS_LONG_LONG;
	ts->ts_scalar.modifiers.sign = SIGN_UNSIGNED;

	return ts;
}

union astnode *create_int(void)
{
	union astnode *ts;

	ALLOC_TYPE(ts, NT_TS_SCALAR);
	ts->ts_scalar.basetype = BT_INT;
	ts->ts_scalar.modifiers.lls = LLS_UNSPEC;
	ts->ts_scalar.modifiers.sign = SIGN_SIGNED;

	return ts;
}

struct addr *gen_rvalue(union astnode *expr, struct addr *dest, enum cc *cc)
{
	struct addr *src1, *src2, *tmp, *tmp2, *tmp3;
	struct quad *quad;
	struct basic_block *tmp_bb;
	enum opcode op;
	enum cc tmp_cc;
	union astnode *ts, *ts_tmp, *iter;

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

		return gen_lvalue(expr, NULL, dest, 0);

	// constant string literal
	case NT_STRING:
		// set up character pointer declaration
		ALLOC_TYPE(ts, NT_TS_SCALAR);
		ts->ts_scalar.basetype = BT_CHAR;
		ts->ts_scalar.modifiers.sign = SIGN_SIGNED;

		ALLOC_TYPE(ts_tmp, NT_DECLSPEC);
		ts_tmp->declspec.ts = ts;

		ALLOC_TYPE(ts, NT_DECLARATOR_ARRAY);
		ts->decl_array.of = ts_tmp;

		// this is kludgey
		ALLOC_TYPE(ts_tmp, NT_NUMBER);
		*((uint64_t*)ts_tmp->num.buf) = 8;
		ts->decl_array.length = ts_tmp;

		// src1 = addr_new(AT_STRING, ts);
		// src1->val.astnode = expr;

		union astnode *decl;
		ALLOC_TYPE(decl, NT_DECL);
		decl->decl.components = ts;
		decl->decl.ident = "TESTSTRING";
		decl->decl.is_string = 1;

		// string is an lvalue!
		return gen_lvalue(decl, NULL, dest, 0);

		// goto constliteral;

	// constant character
	case NT_CHARLIT:
		// assume 1-byte character (i.e., not wide)
		ALLOC_TYPE(ts, NT_TS_SCALAR);
		ts->ts_scalar.basetype = BT_CHAR;
		ts->ts_scalar.modifiers.sign = SIGN_SIGNED;

		src1 = addr_new(AT_CONST, ts);
		*((uint64_t*)src1->val.constval) =
			(uint64_t)expr->charlit.charlit.value.none;

		goto constliteral;

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

		goto constliteral;

	constliteral:
		if (dest) {
			quad_new(OC_MOV, dest, src1, NULL);
		} else {
			dest = src1;
		}
		return dest;

	// fncall
	case NT_FNCALL:
		src1 = gen_rvalue(expr->fncall.fnname, NULL, NULL);

		// make sure src1 is a fn type (includes implicit functions)
		if (NT(src1->decl) != NT_DECLARATOR_FUNCTION) {
			yyerror_fatal("attempting to call a non-function");
			return NULL;
		}

		// generate a struct addr for each argument in fncall arglist
		src2 = tmp = addr_new(AT_CONST, create_size_t());
		LL_FOR(expr->fncall.arglist, iter) {
			tmp->next = gen_rvalue(iter, NULL, NULL);
			tmp = tmp->next;
		}

		if (!dest) {
			// TODO: should infer type from function declaration;
			// 	for now, default return type is int
			dest = tmp_addr_new(create_int());
		}

		quad_new(OC_CALL, dest, src1, src2->next);
		return dest;

	// unary operator
	case NT_UNOP:

		// special cases: sizeof; these do not generate quads and
		// do not demote arrays
		switch (expr->unop.op) {

		// sizeof with a symbol or constexpr
		case SIZEOF:
#if !DEBUG2
			// generate quads for expression in sizeof operand
			// for debugging purposes only; otherwise send to
			// a dummy basic block not linked into the CFG
			tmp_bb = cur_bb;
			cur_bb = dummy_basic_block_new();
#endif
			src1 = gen_rvalue(expr->unop.arg, NULL, NULL);
#if !DEBUG2
			cur_bb = tmp_bb;
#endif

			// src2 is the resultant struct addr (static number
			// after compilation)
			src2 = addr_new(AT_CONST, create_size_t());
			*((uint64_t *) src2->val.constval)
				= src1->type == AT_CONST
				  ? src1->size
				  : astnode_sizeof_type(src1->decl);

			if (dest) {
				quad_new(OC_MOV, dest, src2, NULL);
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
				quad_new(OC_MOV, dest, src1, NULL);
			} else {
				dest = src1;
			}
			return dest;
		}

		// lvalue unops: & (addressof), ++, -- (post inc/dec)
		switch (expr->unop.op) {
		// pointer deref
		case '*':
			// lvalue is the deref expression itself
			return gen_lvalue(expr, NULL, dest, 0);

		// addressof
		case '&':
			// lvalue is the argument to the addressof expression
			return gen_lvalue(expr->unop.arg, NULL, dest, 1);

		// TODO: implement postinc/postdec
		// postincrement/decrement operators
		case PLUSPLUS:
			NYI("postinc");
			return NULL;

		case MINUSMINUS:
			NYI("preinc");
			return NULL;
		}

		// other rvalue unops: these generate quads and demote arrays
		src1 = gen_rvalue(expr->unop.arg, NULL, NULL);
		demote_array(src1);

		switch (expr->unop.op) {

		// logical not
		case '!':
			NYI("logical NOT");
			return NULL;

		// bitwise NOT
		case '~':
			NYI("bitwise NOT");
			return NULL;

		}
		break;

	// binary operator
	case NT_BINOP:
		// special binops: assignment, LOGOR/LOGAND (implicit control
		// flow)
		switch (expr->binop.op) {
		case '=':
			return gen_assign(expr, dest);

		case LOGAND:
			if (!dest && !cc) {
				// this would return a logical
				dest = tmp_addr_new(create_size_t());
			}

			/**
			 * rewrite p&&q => if(p) q else 0;
			 * 
			 * cases (note dest/cc mutually exclusive):
			 * - if no dest/cc specified, create dest, create new
			 * 	temporary as usual
			 * - if dest specified, output to dest as usual
			 * - if cc specified, get condition code of q, make
			 * 	else statement match (e.g., if q sets < cc, then
			 * 	output CMP 1,0)
			 * 
			 * TODO: move this to its own file
			 */
			enum cc tmp_cc;
			struct basic_block *bb_true, *bb_false, *bb_next;
			int cmp_val;

			bb_true = basic_block_new(0);
			bb_false = basic_block_new(0);
			bb_next = basic_block_new(0);

			generate_conditional_quads(expr->binop.left,
				bb_true, bb_false, 1);

			cur_bb = bb_true;
			bb_ll_push(cur_bb);
			if (cc) {
				tmp = gen_rvalue(expr->binop.right, NULL,
					&tmp_cc);
				if (tmp_cc == CC_UNSPEC) {
					tmp2 = addr_new(AT_CONST, create_int());
					*((uint64_t*)tmp2->val.constval) = 0;

					quad_new(OC_CMP, NULL, tmp, tmp2);
					tmp_cc = CC_NE;
				}
			} else {
				gen_rvalue(expr->binop.right, dest, NULL);
			}
			link_bb(CC_ALWAYS, bb_next, NULL);

			cur_bb = bb_false;
			bb_ll_push(cur_bb);
			
			// generate expression that will always be false
			if (cc) {
				switch (tmp_cc) {
				case CC_L:
				case CC_LE:
				case CC_E:
					cmp_val = -1;
					break;
				case CC_G:
				case CC_GE:
					cmp_val = 1;
					break;
				case CC_NE:
					cmp_val = 0;
					break;
				}

				tmp = addr_new(AT_CONST, create_int());
				*((uint64_t*)tmp->val.constval) = 0;
				tmp2 = addr_new(AT_CONST, create_int());
				*((uint64_t*)tmp2->val.constval) = cmp_val;
				quad_new(OC_CMP, NULL, tmp, tmp2);
			} else {
				tmp = addr_new(AT_CONST, create_int());
				*((uint64_t*)tmp->val.constval) = 0;
				quad_new(OC_MOV, dest, tmp, NULL);
			}
			link_bb(CC_ALWAYS, bb_next, NULL);

			cur_bb = bb_next;
			bb_ll_push(cur_bb);

			// return value and set cc
			if (cc) {
				*cc = tmp_cc;
			}
			return dest;

		// TODO: implement LOGOR (same idea as LOGAND)
		case LOGOR:
			NYI("logical OR");
			break;
		}

		src1 = gen_rvalue(expr->binop.left, NULL, NULL);
		src2 = gen_rvalue(expr->binop.right, NULL, NULL);

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

				// implicitly upcast src2
				if (src2->size != 8) {
					tmp3 = tmp_addr_new(create_size_t());
					quad_new(OC_CAST, tmp3, src2, NULL);
					src2 = tmp3;
				}

				tmp2 = tmp_addr_new(create_size_t());
				quad_new(OC_MUL, tmp2, tmp, src2);
				src2 = tmp2;
			}

			op = OC_ADD;
			goto basicop;

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

				// implicitly upcast src2
				if (src2->size != 8) {
					tmp3 = tmp_addr_new(create_size_t());
					quad_new(OC_CAST, tmp3, src2, NULL);
					src2 = tmp3;
				}

				tmp2 = tmp_addr_new(create_size_t());
				quad_new(OC_MUL, tmp2, tmp, src2);
				src2 = tmp2;
			}

			op = OC_SUB;
			goto basicop;

		finishsubop:
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

				quad_new(OC_DIV, dest, tmp2, tmp);
			}

			return dest;

		// explicit type cast
		case 'c':
			// TODO: should check if dest type is compatible

			if (!dest) {
				dest = tmp_addr_new(src1->decl);
			}
			quad_new(OC_CAST, dest, src2, NULL);
			return dest;

		// TODO: implement these
		case SHL:
			NYI("left shift");
			break;

		case SHR:
			NYI("right shift");
			break;

		case '^':
			NYI("bitwise XOR");
			break;

		case '&':
			NYI("bitwise AND");
			break;

		case '|':
			NYI("bitwise OR");
			break;

		// addition and subtraction already taken care of
		// arithmetic operators
		case '*':	op = OC_MUL; goto basicop;
		case '/':	op = OC_DIV; goto basicop;
		case '%':	op = OC_MOD; goto basicop;
		basicop:
			if (!dest) {
				dest = tmp_addr_new(src1->decl);
			}

			// implicit cast to larger type
			if (src1->size < src2->size) {
				tmp = tmp_addr_new(src2->decl);
				quad_new(OC_CAST, src1, tmp, NULL);
				src1 = tmp;
			} else if (src1->size > src2->size) {
				tmp = tmp_addr_new(src1->decl);
				quad_new(OC_CAST, src2, tmp, NULL);
				src2 = tmp;
			}

			// implicit cast to dest type
			if (MAX(src1->size, src2->size) != dest->size) {
				tmp = tmp_addr_new(src1->decl);
				quad_new(op, tmp, src1, src2);
				quad_new(OC_CAST, dest, tmp, NULL);
			} else {
				quad_new(op, dest, src1, src2);
			}

			// subtraction has special hook for pointer - pointer
			if (expr->binop.op == '-') {
				goto finishsubop;
			}

			return dest;

		// relational operators
		case '<':	tmp_cc = CC_L; goto relop;
		case LTEQ:	tmp_cc = CC_LE; goto relop;
		case '>':	tmp_cc = CC_G; goto relop;
		case GTEQ:	tmp_cc = CC_GE; goto relop;
		case EQEQ:	tmp_cc = CC_E; goto relop;
		case NOTEQ:	tmp_cc = CC_NE; goto relop;
		relop:
			// note: dest and cc should be mutually exclusive,
			// but this isn't checked here
			if (!dest && !cc) {
				// create an integer type
				dest = tmp_addr_new(create_size_t());
			}

			quad_new(OC_CMP, dest, src1, src2);

			if (!cc) {
				tmp = addr_new(AT_CONST, create_int());
				*((uint64_t*)tmp->val.constval) = tmp_cc;

				quad_new(OC_SETCC, dest, tmp, NULL);
			} else {
				*cc = tmp_cc;
			}
			return dest;

		// TODO: member access
		case INDSEL:
		case '.':
			NYI("struct/union member access");
			break;
		}
		break;

	default:
		NYI("quadgen: other expression type quad generation");
	}

	yyerror_fatal("quadgen: invalid fallthrough; some expr not handled");
	return NULL;
}

struct addr *gen_lvalue(union astnode *expr, enum addr_mode *mode,
	struct addr *dest, int addrof)
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

		// TODO: check that this is only used in fncalls
		case NT_DECLARATOR_FUNCTION:
			dest = addr_new(AT_AST, expr->decl.components);
			dest->val.astnode = expr;
			return dest;

		case NT_DECLSPEC:
			if (NT(expr->decl.components->declspec.ts) !=
				NT_TS_SCALAR) {
				yyerror_fatal("struct/union lvalue"
					      " not supported (yet?)");
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
					quad_new(OC_MOV, dest, tmp, NULL);
				}
			}
			// addressof
			else {
				if (!dest) {
					dest = tmp_addr_new(create_pointer_to(
						tmp->decl));
				}
				quad_new(OC_LEA, dest, tmp, NULL);
			}

			return dest;

		// treat string as array
		// case NT_STRING:
		// 	// set up character pointer declaration
		// 	ALLOC_TYPE(ts, NT_TS_SCALAR);
		// 	ts->ts_scalar.basetype = BT_CHAR;
		// 	ts->ts_scalar.modifiers.sign = SIGN_SIGNED;

		// 	union astnode *ts_tmp;
		// 	ALLOC_TYPE(ts_tmp, NT_DECLSPEC);
		// 	ts_tmp->declspec.ts = ts;

		// 	ALLOC_TYPE(ts, NT_DECLARATOR_ARRAY);
		// 	ts->decl_array.of = ts_tmp;

		// 	// this is kludgey -- used to get correct sizeof
		// 	ALLOC_TYPE(ts_tmp, NT_NUMBER);
		// 	*((uint64_t*)ts_tmp->num.buf) = 8;
		// 	ts->decl_array.length = ts_tmp;

		// 	union astnode *decl;
		// 	ALLOC_TYPE(decl, NT_DECL);
		// 	decl->decl.components = ts;
		// 	decl->decl.ident = ".STRING";

		// 	expr = decl;

			// src1 = addr_new(AT_STRING, ts);
			// src1->val.astnode = expr;

			// fallthrough

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
				quad_new(OC_LEA, dest, tmp, NULL);
			}
			// addressof
			else {
				if (!dest) {
					dest = tmp_addr_new(create_pointer_to(
						tmp->decl));
				}
				// noop/reinterpret cast
				quad_new(OC_CAST, dest, tmp, NULL);
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
		tmp = gen_rvalue(expr->unop.arg, addrof?dest:NULL, NULL);
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
					quad_new(OC_LOAD, dest, tmp, NULL);
				}
				// pointing to an array (no-op/reinterpret cast)
				else {
					quad_new(OC_CAST, dest, tmp, NULL);
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

struct addr *gen_assign(union astnode *expr, struct addr *target)
{
	struct addr *dest, *src;
	enum addr_mode mode;

	// e.g., in the case of empty for assignment
	if (!expr) {
		return NULL;
	}

	if (NT(expr) != NT_BINOP || expr->binop.op != '=') {
		yyerror_fatal("quadgen: gen_assign(): not an assignment");
		return NULL;
	}

	// generate lvalue; if invalid lvalue, will report and panic in
	// gen_lvalue()
	dest = gen_lvalue(expr->binop.left, &mode, NULL, 0);

	// TODO: don't allow assignment to array/function lvalue

	if (mode == AM_DIRECT) {
		src = gen_rvalue(expr->binop.right, dest, NULL);
	} else {
		src = gen_rvalue(expr->binop.right, NULL, NULL);
		quad_new(OC_STORE, NULL, src, dest);
	}

	// directly being assigned to a memory location, issue a MOV
	if (target) {
		quad_new(OC_MOV, target, src, NULL);
	}

	return src;
}
