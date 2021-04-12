/**
 * Defining the data structures and functions for quads and basic blocks.
 */

#ifndef QUADSH
#define QUADSH

#include <parser/astnode.h>

/**
 * List of opcodes for the quad IR
 */
enum opcode {
	LOAD,
	STORE,
	ADD,
	SUB,
	// TODO
};

/**
 * A single instruction in the quad (3-address) IR.
 */
struct quad {
	// linked list of quads
	// not sure if we'll need prev yet
	struct quad *next, *prev;

	// pointer to containing basic block
	// not sure if we'll need it yet
	struct basic_block *bb;

	enum opcode opcode;
	union astnode *dest, *src1, *src2;
};

/**
 * Holds a linked list of quads (which may be in reverse order when building
 * the basic_block; see generate_quads()), and information about predecessor/
 * successor basic blocks
 *
 * Is uniquely identified by its function name and basic block number within
 * the function.
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

	// conditional to branch on at the end of the BB
	// TODO: should this also be a quad? should this just be the final
	//	quad in the linked list?
	struct quad *cond;
};

/**
 * Generate basic blocks and quads for a function
 *
 * Like complex declarations, basic blocks are built "in reverse" (by nature
 * of a singly-linked list) and then reversed when complete. While having a
 * second pointer would make it easy to build in the correct order, we do away
 * with all the troubles of maintaining extra pointers.
 * 
 * @param fn_decl		declarator for a function definition
 */
struct basic_block *generate_quads(union astnode *fn_decl);

#endif