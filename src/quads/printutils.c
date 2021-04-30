#include <parser/astnode.h>
#include <parser/printutils.h>
#include <quads/printutils.h>
#include <stdio.h>
#include <stdint.h>

char *opcode2str(enum opcode oc)
{
	switch (oc) {
	case OC_LOAD:	return "LOAD";
	case OC_STORE:	return "STORE";
	case OC_LEA:	return "LEA";
	case OC_ADD:	return "ADD";
	case OC_SUB:	return "SUB";
	case OC_MUL:	return "MUL";
	case OC_DIV:	return "DIV";
	case OC_MOV:	return "MOV";
	case OC_CMP:	return "CMP";
	case OC_CALL:	return "CALL";
	case OC_RET:	return "RETURN";
	case OC_SETCC:	return "SETCC";

	// pseudo-opcode
	case OC_CAST:	return "CAST";
	}

	yyerror_fatal("invalid opcode");
}

char *cc2str(enum cc cc) {
	switch (cc) {
	case CC_ALWAYS:	return NULL;
	case CC_E: 	return "E";
	case CC_NE: 	return "NE";
	case CC_L:	return "L";
	case CC_LE:	return "LE";
	case CC_G:	return "G";
	case CC_GE:	return "GE";
	}
}

void print_addr(struct addr *addr)
{
	FILE *fp = stdout;
	unsigned char *constval;

	if (!addr) {
		yyerror_fatal("quadgen: addr should not be NULL"
			" in print_addr()");
		return;
	}

	// indicate size of addr
	fprintf(fp, "[%d:", addr->size);

#if DEBUG2
	// more intense debugging: print all struct addr types
	// probably unnecessary unless trying to debug something
	print_declarator(addr->decl, 0);
	fprintf(fp, ":");
#endif

	switch (addr->type) {

	// print constant (scalar) value (as hex)
	case AT_CONST:
		constval = addr->val.constval;
		fprintf(fp, "const:0x");
		switch (addr->size) {
		case 1: fprintf(fp, "%x",  *((uint8_t *) constval)); break;
		case 2: fprintf(fp, "%x",  *((uint16_t *)constval)); break;
		case 4: fprintf(fp, "%x",  *((uint32_t *)constval)); break;
		case 8: fprintf(fp, "%lx", *((uint64_t *)constval)); break;
		case 16: fprintf(fp, "%lx%lx", *((uint64_t *)constval),
			*((uint64_t *)constval+1)); break;
		default: yyerror_fatal("quadgen: invalid size");
		}
		fprintf(fp, "]");
		break;

	// print temporary value (id)
	case AT_TMP:
		fprintf(fp, "tmp:%%%d]", addr->val.tmpid);
		break;

	// print symbol table value
	case AT_AST:
		fprintf(fp, "symbol:%s]", addr->val.astnode->decl.ident);
		break;

	default:
		yyerror_fatal("quadgen: unknown address type");
	}
}

void print_quad(struct quad *quad)
{
	FILE *fp = stdout;
	struct addr *iter;

	if (!quad) {
		yyerror_fatal("quadgen: quad should not be NULL"
			" in print_quad()");
		return;
	}

	fprintf(fp, "\t");

	// print dest addr (if applicable)
	if (quad->dest) {
		print_addr(quad->dest);
		fprintf(fp, "=");
	}

	// print opcode
	fprintf(fp, "%s ", opcode2str(quad->opcode));

	// print source addr (if applicable)
	// (0- and 1-operand opcodes exist)
	if (quad->src1) {
		print_addr(quad->src1);
		
		if (quad->src2) {
			fprintf(fp, ", ");

			// regular opcodes
			if (quad->opcode != OC_CALL) {
				print_addr(quad->src2);
			}
			// fncall opcode: ll of fncall arglist
			else {
				fprintf(fp, "(arglist");
				_LL_FOR(quad->src2, iter, next) {
					fprintf(fp, " ");
					print_addr(iter);
				}
				fprintf(fp, ")");
			}
		}
	}
	
	fprintf(fp, "\n");
}

// TODO: remove this; don't need this because we want to print out basic blocks
// in reversed order
// // (temporary?) duplicate checker; keeps track of seen basic blocks
//static struct bb_history {
//	struct bb_history *next;
//	struct basic_block *bb;
//} *bb_history;

void print_basic_block(struct basic_block *bb)
{
	FILE *fp = stdout;
	struct quad *iter;
	struct bb_history *hist_iter, *cur;

	// shouldn't happen
	if (!bb) {
		yyerror_fatal("quadgen: null bb in print_basic_block");
		return;
	}

	// TODO: remove
//	// check if basic block has already been seen
//	_LL_FOR(bb_history, hist_iter, next) {
//		if (hist_iter->bb == bb) {
//			return;
//		}
//	}
//
//	// allocate new history item
//	cur = calloc(1, sizeof(struct bb_history));
//	cur->bb = bb;
//	cur->next = bb_history;
//	bb_history = cur;

	// print bb identifier
	fprintf(fp, ".BB.%s.%d {\n", bb->fn_name, bb->bb_no);

	// print all quads in bb
	_LL_FOR(bb->ll, iter, next) {
		print_quad(iter);
	}

	// print what it branches to
	// conditional branch (if exists)
	if (bb->next_cond && bb->branch_cc != CC_ALWAYS) {
		fprintf(fp, "\tJMP%s .BB.%s.%d\n", cc2str(bb->branch_cc),
			bb->next_cond->fn_name, bb->next_cond->bb_no);
	}
	// unconditional branch (if exists; doesn't exist for final
	// bb in function, i.e., CFG terminal node)
	if (bb->next_def) {
		fprintf(fp, "\tJMP .BB.%s.%d\n",
			bb->next_def->fn_name, bb->next_def->bb_no);
	} else {
		fprintf(fp, "\t(CFG terminal node)\n");
	}

	fprintf(fp, "}\n");

//	// print children
//	print_basic_block(bb->next_def);
//	print_basic_block(bb->next_cond);

	// TODO: remove
//	switch(bb->branch){
//		case NEVER:		break;
//		case ALWAYS:	fprintf(fp, "BR .BB.%s.%d\n", bb->prev->fn_name, bb->prev->bb_no);
//		// case BR_LT:		fprintf(fp, "BRLT .BB.%s.%d, .BB.%s.%d\n", bb->prev.fn_name, bb->prev->bb_no, bb->next_cond.fn_name, bb->next_cond.bb_no);
//		// case BR_GT:		fprintf(fp, "BRGT .BB.%s.%d, .BB.%s.%d\n", bb->prev.fn_name, bb->prev.bb_no, bb->next_cond.fn_name, bb->next_cond.bb_no);
//		// case BR_EQ:		fprintf(fp, "BREQ .BB.%s.%d, .BB.%s.%d\n", bb->prev.fn_name, bb->prev.bb_no, bb->next_cond.fn_name, bb->next_cond.bb_no);
//		// case BR_NEQ:	fprintf(fp, "BRNE .BB.%s.%d, .BB.%s.%d\n", bb->prev.fn_name, bb->prev.bb_no, bb->next_cond.fn_name, bb->next_cond.bb_no);
//		// case BR_LTEQ:	fprintf(fp, "BRLE .BB.%s.%d, .BB.%s.%d\n", bb->prev.fn_name, bb->prev.bb_no, bb->next_cond.fn_name, bb->next_cond.bb_no);
//		// case BR_GTEQ:	fprintf(fp, "BRGE .BB.%s.%d, .BB.%s.%d\n", bb->prev.fn_name, bb->prev.bb_no, bb->next_cond.fn_name, bb->next_cond.bb_no);
//
//	}


}

// TODO: add documentation
// 	in documentation: this should only happen after finalize_bb_list
void print_basic_blocks(struct basic_block *bb)
{
	struct basic_block *iter;

	_LL_FOR(bb_ll, iter, next) {
		print_basic_block(iter);
	}

	// TODO: remove
//	// print
//	print_basic_block(bb);
//	print_basic_blocks(bb->next_def);
//	print_basic_blocks(bb->next_cond);
}
