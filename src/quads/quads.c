#include <quads/quads.h>
#include <quads/printutils.h>
#include <quads/sizeof.h>
#include <string.h>
#include <stdint.h>
#include <parser.tab.h>

// predeclaring some local functions
static struct addr *gen_assign(union astnode *expr,
	struct addr *target, struct basic_block *bb);

// current function name and basic block number
static char *fn_name;
static int bb_no, tmp_no;
struct loop *cur_loop;

/**
 * TODO: add description to all these local (static) functions
 */
static struct basic_block *basic_block_new()
{
	struct basic_block *bb = calloc(1, sizeof(struct basic_block));

	// set basic block identifier
	bb->fn_name = strdup(fn_name);
	bb->bb_no = bb_no++;

	return bb;
}

/**
 * emits a new quad to the specified basic block
 * 
 * any of the operands or src may be null, depending on the opcode
 * 
 * @param bb		basic block to emit quad to
 * @param opcode	quad opcode
 * @param dest		quad destination; must be an lvalue
 * @param src1		quad first operand
 * @param src2		quad second operand
 * @return		generated quad
 */
static struct quad *quad_new(struct basic_block *bb, enum opcode opcode,
	struct addr *dest, struct addr *src1, struct addr *src2)
{
	struct quad *quad = calloc(1, sizeof(struct quad));

	*quad = (struct quad) {
		.bb = bb,
		.next = bb->ll,

		.opcode = opcode,
		.dest = dest,
		.src1 = src1,
		.src2 = src2,
	};

	// note: this generates the basic block in reverse
	bb->ll = quad;

	return quad;
}

/**
 * constructs and returns a new struct addr (operand/dest to quad)
 * 
 * @param type		type of addr (memory (variable), immediate (constant),
 * 			or temporary (register))
 * @param decl		astnode representation of the type of the value
 * @return		constructed struct addr
 */
static struct addr *addr_new(enum addr_type type, union astnode *decl)
{
	struct addr *addr = calloc(1, sizeof(struct addr));

	*addr = (struct addr) {
		.type = type,
		.size = astnode_sizeof_type(decl),
		.decl = decl,
	};

	return addr;
}

/**
 * constructs and returns a new temporary pseudo-register (for subexpressions)
 * 
 * also gives the struct addr a unique ID
 * 
 * @param decl		astnode representation of the type of the value
 * @return		constructed struct addr
 */
static struct addr *tmp_addr_new(union astnode *decl)
{
	struct addr *addr = addr_new(AT_TMP, decl);

	// assign unique temporary (pseudo-)register identifier
	addr->val.tmpid = tmp_no++;

	return addr;
}

/**
 * iteratively and recursively generates a linked-list of quads for an
 * (r-value) expression; corresponds to function of same name in lecture notes
 * 
 * The use of the target address is to have more optimized 3-address quads,
 * i.e., remove extra MOV quads. (These will still have to be added back in
 * when we get to assembly code generation, as x86 is a 2-address architecture.)
 * 
 * semantic notes:
 * - if primary expression (scalar or constant):
 * 	- if no target, return as a struct addr
 * 	- if target is specified, emit MOV opcode
 * - if assignment (=):
 * 	- evaluate left lvalue (and check that it is an lvalue)
 * 	- evaluate right rvalue with target set to left lvalue
 * 	- if dest set, emit MOV quad from lvalue (arbitrarily chosen) to dest
 * 		(this happens if multiple assignments directly in a row)
 * - if unary/binary expression, emit quad
 * 	- if dest is NULL, a temporary is created
 *	- if target, it is the destination of the quad
 *
 * @param expr		expression object
 * @param dest		target address, or NULL if temporary expression
 * @param bb		basic block to add quads to
 * @return 		addr storing result of expression
 */
static struct addr *gen_rvalue(union astnode *expr,
	struct addr *dest, struct basic_block *bb)
{
	struct addr *src1, *src2;
	union astnode *ts;
	enum opcode op;

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

		src1 = addr_new(AT_AST, expr->decl.components);
		src1->val.astnode = expr;

		if (dest) {
			quad_new(bb, OC_MOV, dest, src1, NULL);
		} else {
			dest = src1;
		}
		return dest;

	// constant number
	case NT_NUMBER:
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
		// TODO: note that sizeof returns type size_t, defined in
		// 	stddef.h -- should represent this type somewhere
		case SIZEOF:
			// create 8-byte type like size_t (similar to
			// unsigned long long in 64-bit systems)
			ALLOC_TYPE(ts, NT_TS_SCALAR);
			ts->ts_scalar.basetype = BT_INT;
			ts->ts_scalar.modifiers.lls = LLS_LONG_LONG;
			ts->ts_scalar.modifiers.sign = SIGN_UNSIGNED;

			src2 = addr_new(AT_CONST, ts);
			*((uint64_t *)src2->val.constval)
				= src1->type == AT_CONST
				? src1->size
				: astnode_sizeof_symbol(expr->unop.arg);
			
			if (dest) {
				quad_new(bb, OC_MOV, dest, src2, NULL);
			} else {
				dest = src2;
			}
			return dest;

		// sizeof with a typename (addr1 should be NULL)
		case 's':
			// see notes above
			ALLOC_TYPE(ts, NT_TS_SCALAR);
			ts->ts_scalar.basetype = BT_INT;
			ts->ts_scalar.modifiers.lls = LLS_LONG_LONG;
			ts->ts_scalar.modifiers.sign = SIGN_UNSIGNED;

			src1 = addr_new(AT_CONST, ts);
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
			// check that the rvalue is a pointer type
			if (NT(src1->decl) != NT_DECLARATOR_POINTER) {
				yyerror_fatal("dereferencing non-pointer type");
			}

			if (!dest) {
				// get the type that the pointer is pointing to
				ts = expr->unop.arg->decl_pointer.of;
				dest = tmp_addr_new(ts);
			}

			quad_new(bb, OC_LOAD, dest, src1, NULL);
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

		// TODO: +/- have to correctly implement pointer arithmetic
		// 	(have to check type of their operands, which means
		// 	that we have to associate type with struct addrs)
		switch (expr->binop.op) {
		// arithmetic
		case '+':
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
static struct addr *gen_lvalue(union astnode *expr,
	struct basic_block *bb, enum addr_mode *mode)
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
		if (NT(dest->decl) != NT_DECLARATOR_POINTER) {
			yyerror_fatal("dereferencing non-pointer type");
		}

		return dest;
	}

	yyerror_fatal("quadgen: gen_lvalue fallthrough: invalid lvalue");
	return NULL;
}

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
static struct addr *gen_assign(union astnode *expr,
	struct addr *target, struct basic_block *bb)
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


/*Loops*/

/*Function to allocate new loop*/
struct loop *loop_new(void)
{
	struct loop *lp = malloc(sizeof(struct loop));
	lp->prev = cur_loop;
	return lp;

	
}



static void generate_while_quads(union astnode *stmt, struct basic_block *bb)
{
	struct basic_block *bb_cond = basic_block_new();
	struct basic_block *bb_body = basic_block_new();
	struct basic_block *bb_next = basic_block_new();

	//Continue and break points
	bb->next_cond = bb_cond;
	bb = bb_cond;

	cur_loop = loop_new();
	cur_loop->bb_cont = bb_cond;
	
	
	//generate_conditional_quads(stmt->stmt_while.cond, )
	
}

/**
 * generate quads for if else statements
 *
 * @param expr		expression in if
 * @param bb		expression in if
 */
static void generate_if_else_quads(union astnode *expr, struct basic_block *bb)
{
	struct basic_block *Bt = basic_block_new();
	struct basic_block *Bf = basic_block_new();
	struct basic_block *Bn;

	if(expr->stmt_if_else.elsestmt)
		Bn = basic_block_new();
	else
		Bn = Bf;

	generate_conditional_quads(expr->stmt_if_else.ifstmt, bb, Bt, Bf);
	
}

/**
 * generate quads for conditional expression
 *
 * @param expr		expression in if
 * @param bb		current basic block
 * @param Bt		basic block Then
 * @param Bf		basic block False
 */
static void generate_conditional_quads(union astnode *expr, struct basic_block *bb, struct basic_block *Bt, struct basic_block *Bf)
{

	switch(expr->generic.type)
	{
		case NT_BINOP:;
			struct addr *addr1 = gen_rvalue(expr->binop.left, NULL, bb);
			struct addr *addr2 = gen_rvalue(expr->binop.left, NULL, bb);
			quad_new(bb, OC_CMP, NULL, addr1, addr2);
			switch(expr->binop.op)
			{
				case '<':	break;
				case '>':	break;
				case LTEQ:	break;
				case GTEQ:	break;
				case EQEQ:	break;
				case NOTEQ:	break;
				default:	;
			}
			break;
		
			
	}

}



/**
 * iteratively and recursively generates a linked-list of basic blocks and quads
 *
 * @param stmt		linked list of statements to generate quads for
 * @param bb		current basic block
 */
static void generate_quads_rec(union astnode *stmt, struct basic_block *bb)
{
	struct basic_block *new_bb;

	// end of recursion
	if (!stmt) {
		return;
	}

	// error
	if (!bb) {
		yyerror_fatal("quadgen: bb bb cannot be NULL");
		return;
	}

	switch (NT(stmt)) {

	// compound statement: continue generating quads from body
	case NT_STMT_COMPOUND:
		generate_quads_rec(stmt->stmt_compound.body, bb);
		break;

	// expression statement: break down into subexpressions
	case NT_STMT_EXPR:
		gen_rvalue(stmt->stmt_expr.expr, NULL, bb);
		break;

	// label statements: declare a new bb
	case NT_STMT_LABEL:
		new_bb = basic_block_new();

		// TODO: associate label/case astnode with this bb

		// TODO: name basic block with the label, not a number

		bb->next_def = new_bb;
		bb = new_bb;

		// TODO: make labelled statements flat? matches syntax better
		// 	that way; this also doesn't work correctly with multiple
		// 	nested labels
//		generate_quads_rec(stmt->stmt_label.body, bb);
		break;

	// unconditional jump statements; terminate current basic block
	// (but have to keep going in case of labels further on)
	case NT_STMT_RETURN:
	case NT_STMT_CONT:
	case NT_STMT_BREAK:
	case NT_STMT_GOTO:
		NYI("unconditional jump statement quad generation");
		break;

	// conditional jump statement
	// TODO: also ternary?
	case NT_STMT_IFELSE:
		NYI("conditional jump statement quad generation");
		break;

	// loops
	case NT_STMT_FOR:
	case NT_STMT_WHILE:
	case NT_STMT_DO_WHILE:
		NYI("loop quad generation");
		break;

	default:
		// TODO: are any statement types missed?
		NYI("other stmt types quad generation");
	}

	// iterate
	generate_quads_rec(LL_NEXT(stmt), bb);
}

struct basic_block *generate_quads(union astnode *fn_decl)
{
	struct basic_block *fn_bb;

	// new function was just declared, update identifiers
	fn_name = strdup(fn_decl->decl.ident);
	bb_no = 0;

	// create starting basic block of function
	fn_bb = basic_block_new();

	// recursively generate quads for each statement
	generate_quads_rec(fn_decl->decl.fn_body, fn_bb);

#if DEBUG
	// dump basic blocks
	print_basic_blocks(fn_bb);
#endif

	return fn_bb;
}