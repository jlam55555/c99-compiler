#include <quads/quads.h>
#include <quads/printutils.h>
#include <quads/sizeof.h>
#include <string.h>
#include <stdint.h>

// current function name and basic block number
static char *fn_name;
static int bb_no, tmp_no;

static struct basic_block *basic_block_new()
{
	struct basic_block *bb = calloc(1, sizeof(struct basic_block));

	// set basic block identifier
	bb->fn_name = strdup(fn_name);
	bb->bb_no = bb_no++;

	return bb;
}

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

static struct addr *addr_new(enum addr_type type, unsigned size)
{
	struct addr *addr = calloc(1, sizeof(struct addr));

	*addr = (struct addr) {
		.type = type,
		.size = size,
	};

	return addr;
}

static struct addr *tmp_addr_new(unsigned size)
{
	struct addr *addr = addr_new(AT_TMP, size);

	// assign unique temporary (pseudo-)register identifier
	addr->val.tmpid = tmp_no++;

	return addr;
}

/**
 * iteratively and recursively generates a linked-list of quads for an
 * expression
 *
 * @param expr		expression object
 * @param bb		basic block to add quads to
 * @return 		addr storing result of expression
 */
static struct addr *generate_expr_quads(union astnode *expr,
	struct basic_block *bb)
{
	struct addr *addr1, *addr2, *addr3;

	// null expression
	// this shouldn't happen but this is here as a safety measure
	if (!expr) {
		yyerror("quadgen: empty expression in generate_expr_quads()");
		return NULL;
	}

	switch (NT(expr)) {

	// symbol
	case NT_DECL:
		// TODO: take sizeof astnode
		addr1 = addr_new(AT_AST, astnode_sizeof_symbol(expr));
		addr1->val.astnode = expr;
		return addr1;

	// constant number
	case NT_NUMBER:
		// TODO: for now, assume 8-byte integer

		// convert into const
		addr1 = addr_new(AT_CONST, 8);
		*addr1->val.constval = (uint64_t) expr->num.num.int_val;
		return addr1;

	// binary operator
	case NT_BINOP:
		addr1 = generate_expr_quads(expr->binop.left, bb);
		addr2 = generate_expr_quads(expr->binop.right, bb);

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
			addr3 = tmp_addr_new(8);
			quad_new(bb, OC_ADD, addr3, addr1, addr2);
			return addr3;
		case '-':
			addr3 = tmp_addr_new(8);
			quad_new(bb, OC_SUB, addr3, addr1, addr2);
			return addr3;

		// assignment
		case '=':
			// TODO: check that addr1 is an lvalue

			quad_new(bb, OC_MOV, addr1, addr2, NULL);

			// can return either addr1 or addr2; either should hold
			// the same value after the MOV opcode
			return addr1;
		}
		break;

	default:
		NYI("other expression type quad generation");
	}

	return NULL;
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
		generate_expr_quads(stmt->stmt_expr.expr, bb);
		break;

	// label statements: declare a new bb
	case NT_STMT_LABEL:
	case NT_STMT_CASE:
		new_bb = basic_block_new();

		// TODO: associate label/case astnode with this bb

		bb->next_def = new_bb;
		bb = new_bb;
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