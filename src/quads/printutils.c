#include <quads/printutils.h>
#include <stdio.h>
#include <stdint.h>

char *opcode2str(enum opcode oc)
{
	switch (oc) {
	case OC_LOAD:	return "LOAD";
	case OC_STORE:	return "STORE";
	case OC_ADD:	return "ADD";
	case OC_SUB:	return "SUB";
	case OC_MOV:	return "MOV";
	}

	yyerror_fatal("invalid opcode");
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
		default: yyerror_fatal("quadgen: invalid size");
		}
		fprintf(fp, "]");
		break;

	// print temporary value (id)
	case AT_TMP:
		fprintf(fp, "tmp:%d]", addr->val.tmpid);
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

	if (!quad) {
		yyerror_fatal("quadgen: quad should not be NULL"
			" in print_quad()");
		return;
	}

	// print dest addr (if applicable)
	if (quad->dest) {
		print_addr(quad->dest);
		fprintf(fp, "=");
	}

	// print opcode
	fprintf(fp, "%s ", opcode2str(quad->opcode));

	// print source addr (if applicable)
	print_addr(quad->src1);
	if (quad->src2) {
		fprintf(fp, ", ");
		print_addr(quad->src2);
	}

	fprintf(fp, "\n");
}

void print_basic_block(struct basic_block *bb)
{
	struct quad *iter;
	FILE *fp = stdout;

	if (!bb) {
		yyerror_fatal("quadgen: basic block should not be NULL"
			" in print_basic_block()");
		return;
	}

	fprintf(fp, ".BB.%s.%d\n", bb->fn_name, bb->bb_no);

	_LL_FOR(bb->ll, iter, next) {
		print_quad(iter);
	}
}

void print_basic_blocks(struct basic_block *bb)
{
	// TODO: make sure not to repeat basic blocks

	if (!bb) {
		return;
	}

	print_basic_block(bb);

	print_basic_blocks(bb->next_def);
	print_basic_blocks(bb->next_cond);
}
