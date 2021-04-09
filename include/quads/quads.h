/**
 * Defining the data structures and functions for quads and basic blocks.
 */

#ifndef QUADSH
#define QUADSH

#include <parser/astnode.h>

/**
 * TODO: desc
 */
enum opcode {
	LOAD
};

/**
 * TODO: desc
 */
struct quad {
	// linked list of quads
	struct quad *next, *prev;

	// pointer to containing basic block
	// not sure if we'll need it yet
	struct basic_block *bb;

	enum opcode opcode;
	union astnode *dest, *src1, *src2;
};

/**
 * TODO: desc
 */
struct basic_block {
	struct quad *ll;

	// basic block identifier
	char *fn_name;
	int bb_no;

	// prev is predecessor BB
	// next_def is default (fall-through) BB
	// next_cond is non-default (conditional) BB
	struct basic_block *prev, *next_def, *next_cond;
};

/**
 * Generate basic blocks/quads for a function
 * 
 * @param fn_decl		declarator for a function definition
 */
struct basic_block *generate_quads(union astnode *fn_decl);

#endif