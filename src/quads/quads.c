#include <quads/quads.h>
#include <string.h>

// current function name and basic block number
static char *fn_name;
static int bb_no;

static struct basic_block *basic_block_new()
{
	struct basic_block *bb = calloc(1, sizeof(struct basic_block));

	// set basic block identifier
	bb->fn_name = strdup(fn_name);
	bb->bb_no = bb_no++;

	return bb;
}

struct basic_block *generate_quads(union astnode *fn_decl)
{
	// TODO
}

void print_basic_block(struct basic_block *bb)
{
	// TODO
}