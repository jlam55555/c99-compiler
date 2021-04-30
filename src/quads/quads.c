#include <quads/quads.h>
#include <quads/printutils.h>
#include <quads/sizeof.h>
#include <quads/exprquads.h>
#include <quads/cfquads.h>
#include <string.h>

struct basic_block *cur_bb, *bb_ll;

// current function name and basic block number
static char *fn_name;
static int bb_no, tmp_no;

struct basic_block *basic_block_new(int add_to_ll)
{
	struct basic_block *bb = calloc(1, sizeof(struct basic_block));

	// set basic block identifier
	bb->fn_name = strdup(fn_name);
	bb->bb_no = bb_no++;

	if (add_to_ll) {
		bb_ll_push(bb);
	}

	return bb;
}

void bb_ll_push(struct basic_block *bb)
{
	bb->next = bb_ll;
	bb_ll = bb;
}

struct basic_block *dummy_basic_block_new(void)
{
	struct basic_block *bb = calloc(1, sizeof(struct basic_block));

	// don't add to bb_ll, give it obviously invalid ID
	bb->bb_no = -1;
	return bb;
}

struct quad *quad_new(enum opcode opcode, struct addr *dest, struct addr *src1,
	struct addr *src2)
{
	struct quad *quad;

	// cannot generate quads (even to be pedantic) because we reverse the
	// quad ll when it is finalized, so this will generate quads out of
	// order; so this is a (necessary) optimization; if we really wanted to
	// be consistent can generate quads to a dummy bb that would not be
	// linked anywhere in the CFG
	if (cur_bb->finalized) {
		// error is non-fatal, but we will not generate quads
		yyerror("unreachable point; not generating quad");
		return NULL;
	}

	quad = calloc(1, sizeof(struct quad));
	*quad = (struct quad) {
		.bb = cur_bb,
		.next = cur_bb->ll,

		.opcode = opcode,
		.dest = dest,
		.src1 = src1,
		.src2 = src2,
	};

	// note: this generates the basic block in reverse; will eventually
	// be re-reversed in link_bb()
	cur_bb->ll = quad;

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

// TODO: convert this so that it returns the basic block that it creates
void gen_stmt_quads(union astnode *stmt)
{

	// end of recursion
	if (!stmt) {
		return;
	}

	switch (NT(stmt)) {

	// compound statement: continue generating quads from body
	case NT_STMT_COMPOUND:
		gen_stmt_quads(stmt->stmt_compound.body);
		break;

	// expression statement: break down into subexpressions
	case NT_STMT_EXPR:
		// TODO: warn if no side-effects (i.e., statement is useless)

		gen_rvalue(stmt->stmt_expr.expr, NULL, NULL);
		break;

	// label statements: declare a new bb
	case NT_STMT_LABEL:
		NYI("generic label statements");
		break;

//		new_bb = basic_block_new();
//
//		// TODO: associate label/case astnode with this bb
//
//		// TODO: name basic block with the label, not a number
//
//		bb->next_def = new_bb;
//		bb = new_bb;
//
//		// TODO: make labelled statements flat? matches syntax better
//		// 	that way; this also doesn't work correctly with multiple
//		// 	nested labels
////		gen_stmt_quads(stmt->stmt_label.body, bb);
//		break;

	// unconditional jump statements; terminate current basic block
	// (but have to keep going in case of labels further on)
	case NT_STMT_RETURN:
		gen_ret_quads(stmt);
		break;

	case NT_STMT_CONT:
	case NT_STMT_BREAK:
		gen_jmp_quads(stmt);
		break;

	case NT_STMT_GOTO:
		NYI("unconditional jump statement quad generation");
		break;

	// conditional jump statement
	// TODO: also ternary?
	case NT_STMT_IFELSE:
		generate_if_else_quads(stmt);
		break;

	// loops
	case NT_STMT_FOR:
		generate_for_quads(stmt);
		break;

	case NT_STMT_WHILE:
		generate_while_quads(stmt);
		break;

	case NT_STMT_DO_WHILE:
		generate_do_while_quads(stmt);
		break;

	default:
		// TODO: are any statement types missed?
		NYI("other stmt types quad generation");
	}

	// iterate
	gen_stmt_quads(LL_NEXT(stmt));
}

// reverse basic blocks
// TODO: document this
static void finalize_bb_list(void)
{
	struct basic_block *a, *b, *c;

	if (!(a = bb_ll) || !(b = _LL_NEXT(a, next))) {
		return;
	}

	c = _LL_NEXT(b, next);
	_LL_NEXT(a, next) = NULL;

	while (c) {
		_LL_NEXT(b, next) = a;
		a = b;
		b = c;
		c = _LL_NEXT(c, next);
	}

	_LL_NEXT(b, next) = a;
	bb_ll = b;
}

struct basic_block *generate_quads(union astnode *fn_decl)
{
	struct basic_block *fn_bb;
	struct addr *tmp;

	// new function was just declared, update identifiers
	fn_name = strdup(fn_decl->decl.ident);
	bb_no = 0;

	// clear basic block linked list
	bb_ll = NULL;

	// create starting basic block of function
	cur_bb = fn_bb = basic_block_new(1);

	// recursively generate quads for each statement
	gen_stmt_quads(fn_decl->decl.fn_body);

	// add implicit return here
	// assume returns an integer type, implicit return 0;
	if (!cur_bb->ll || cur_bb->ll->opcode != OC_RET) {
		tmp = addr_new(AT_CONST, create_int());
		*((uint64_t*)tmp->val.constval) = 0;

		quad_new(OC_RET, NULL, tmp, NULL);
	}

	// need to call this to finalize the last BB (i.e., reverse its quads)
	link_bb(CC_ALWAYS, NULL, NULL);

	// finalize bb list by reversing the order
	finalize_bb_list();

#if DEBUG
	// dump basic blocks (recursively print out CFG)
	print_basic_blocks();
#endif

	return fn_bb;
}