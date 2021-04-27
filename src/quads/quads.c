#include <quads/quads.h>
#include <quads/printutils.h>
#include <quads/sizeof.h>
#include <quads/exprquads.h>
#include <string.h>
#include <stdint.h>
#include <parser.tab.h>

// predeclaring some local functions
static void generate_quads_rec(union astnode *stmt, struct basic_block *bb);

// current function name and basic block number
static char *fn_name;
static int bb_no, tmp_no;
struct loop *cur_loop;

struct basic_block *basic_block_new(void)
{
	struct basic_block *bb = calloc(1, sizeof(struct basic_block));

	// set basic block identifier
	bb->fn_name = strdup(fn_name);
	bb->bb_no = bb_no++;
	return bb;
}

struct basic_block *dummy_basic_block_new(void)
{
	struct basic_block *bb = calloc(1, sizeof(struct basic_block));

	bb->bb_no = -1;
	return bb;
}

struct quad *quad_new(struct basic_block *bb, enum opcode opcode,
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

struct addr *addr_new(enum addr_type type, union astnode *decl)
{
	struct addr *addr = calloc(1, sizeof(struct addr));

	*addr = (struct addr) {
		.type = type,
		.size = astnode_sizeof_type(decl),
		.decl = decl,
	};

	return addr;
}

struct addr *tmp_addr_new(union astnode *decl)
{
	struct addr *addr = addr_new(AT_TMP, decl);

	// assign unique temporary (pseudo-)register identifier
	addr->val.tmpid = tmp_no++;

	return addr;
}

/*Loops*/

/*Function to allocate new loop*/
struct loop *loop_new(void)
{
	struct loop *lp = malloc(sizeof(struct loop));
	lp->prev = cur_loop;
	return lp;

	
}

static void generate_for_quads(union astnode *stmt, struct basic_block *bb)
{

	struct basic_block *bb_cond = basic_block_new();
	struct basic_block *bb_body = basic_block_new();
	struct basic_block *bb_update = basic_block_new();
	struct basic_block *bb_next = basic_block_new();

	cur_loop = loop_new();
	//Continue and break points
	cur_loop->bb_cont = bb_cond;
	cur_loop->bb_break = bb_next;

	//gen_assign(stmt->stmt_for.init);

	

	
}



static void generate_do_while_quads(union astnode *stmt, struct basic_block *bb)
{
	struct basic_block *bb_cond = basic_block_new();
	struct basic_block *bb_body = basic_block_new();
	struct basic_block *bb_next = basic_block_new();

	cur_loop = loop_new();
	//Continue and break points
	cur_loop->bb_cont = bb_cond;
	cur_loop->bb_break = bb_next;

	link_basic_block(bb, ALWAYS, bb_body, NULL);
	bb = bb_body;

	generate_quads_rec(stmt->stmt_while.body, bb);

	link_basic_block(bb, ALWAYS, bb_cond, NULL);
	bb = bb_cond;

	generate_conditional_quads(stmt->stmt_while.cond, bb, bb_body, bb_next);

	bb = bb_next;
	cur_loop = cur_loop->prev;


}


static void generate_while_quads(union astnode *stmt, struct basic_block *bb)
{
	struct basic_block *bb_cond = basic_block_new();
	struct basic_block *bb_body = basic_block_new();
	struct basic_block *bb_next = basic_block_new();


	cur_loop = loop_new();
	//Continue and break points
	cur_loop->bb_cont = bb_cond;
	cur_loop->bb_break = bb_next;

	link_basic_block(bb, ALWAYS, bb_cond, NULL);
	bb = bb_cond;

	generate_conditional_quads(stmt->stmt_while.cond, bb, bb_body, bb_next);
	
	bb = bb_body;
	generate_quads_rec(stmt->stmt_while.body, bb);

	link_basic_block(bb, ALWAYS, bb_cond, NULL);

	bb = bb_next;
	cur_loop = cur_loop->prev;
	
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

	bb = Bt;
	
	generate_quads_rec(expr->stmt_if_else.ifstmt, bb);
	link_basic_block(bb, ALWAYS, Bn, NULL);

	if(expr->stmt_if_else.elsestmt)
	{
		bb = Bf;
		generate_quads_rec(expr->stmt_if_else.elsestmt, bb);
		link_basic_block(bb, ALWAYS, Bn, NULL);
		
	}

	bb = Bn;

	
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
			struct addr *addr2 = gen_rvalue(expr->binop.right, NULL, bb);
			quad_new(bb, OC_CMP, NULL, addr1, addr2);

			// struct addr *addr_bt = gen
			switch(expr->binop.op)
			{
				case '<':	link_basic_block(bb, BR_LT, Bt, Bf);	break;
				case '>':	link_basic_block(bb, BR_GT, Bt, Bf);	break;
				case LTEQ:	link_basic_block(bb, BR_LTEQ, Bt, Bf);	break;
				case GTEQ:	link_basic_block(bb, BR_GTEQ, Bt, Bf);	break;
				case EQEQ:	link_basic_block(bb, BR_EQ, Bt, Bf);	break;
				case NOTEQ:	link_basic_block(bb, BR_NEQ, Bt, Bf);	break;
				default:	yyerror("binop in conditional expr\n");
			}
			break;
		
			
	}

}

static void generate_cont_break_quads(union astnode *stmt, struct basic_block *bb)
{
	if(NT(stmt))
		link_basic_block(bb, ALWAYS, cur_loop->bb_break, NULL);
	else
	{
		if(NT(stmt))
			link_basic_block(bb, ALWAYS, cur_loop->bb_cont, NULL);
	}
}


struct basic_block *link_basic_block(struct basic_block *bb, enum branches branch, struct basic_block *prev, struct basic_block *next)
{
	bb->branch = branch;
	bb->prev = prev;
	bb->next_cond = next;

	return bb;
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
		// TODO: warn if no side-effects (i.e., statement is useless)

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
		break;
	case NT_STMT_CONT:
	case NT_STMT_BREAK:
		break;
	case NT_STMT_GOTO:
		NYI("unconditional jump statement quad generation");
		break;

	// conditional jump statement
	// TODO: also ternary?
	case NT_STMT_IFELSE:
		generate_if_else_quads(stmt, bb);
		break;

	// loops
	case NT_STMT_FOR:
		generate_for_quads(stmt, bb);
		break;
	case NT_STMT_WHILE:
		generate_while_quads(stmt, bb);
		break;
	case NT_STMT_DO_WHILE:
		generate_do_while_quads(stmt, bb);
		break;

	default:
		// TODO: are any statement types missed?
		NYI("other stmt types quad generation");
	}

	// iterate
	generate_quads_rec(LL_NEXT(stmt), bb);
}

/**
 * begin generating quads at the top-level (function level)
 * 
 * @param fn_decl	function declaration to generate quads for
 * @return		basic block CFG
 */
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