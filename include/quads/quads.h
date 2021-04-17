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
	OC_LOAD,
	OC_STORE,
	OC_ADD,
	OC_SUB,
	OC_MOV,
	OC_MUL,
	OC_DIV,
	OC_MOD,
	OC_AND,
	OC_OR,
	OC_XOR,
	OC_SHL,
	OC_SHR,
	OC_CMP,
	OC_LT,
	OC_GT,
	OC_LE,
	OC_GE,
	OC_EQ,
	OC_NEQ,
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
	struct addr *dest, *src1, *src2;
};

/**
 * An entity that may be used as the source or operand of a quad.
 *
 * Can store scalar variables (astnode_decl objects, temporaries, or constant
 * values).
 *
 * Note: an assumption is made that the register size is 64 bits, and thus
 * a constant (scalar) has maximum size 64 bits.
 */
struct addr {
	// reference to astnode, temporary (pseudo-register), constant value
	enum addr_type { AT_AST, AT_TMP, AT_CONST } type;

	// size in bytes of the operand; since we're only dealing with scalar
	// types here, this should never exceed the size of the arch. register
	// size (e.g., 8 bytes on an x86_64 arch.)
	unsigned size;

	// data associated with each type
	union addr_val {
		union astnode *astnode;
		unsigned char constval[8];
		unsigned tmpid;
	} val;

	// TODO: will probably need to associate each struct addr instance
	// 	with a type, especially in the case of casting, or finding the
	// 	type of deeply nested expressions, or determining l/rvalues
	union astnode *decl;
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

/**
 * generate quads for if else statements
 *
 * @param expr		expression in if
 */
static void generate_if_else_quads(union astnode *expr);

/**
 * generate quads for conditional expression
 *
 * @param expr		expression in if
 * @param Bt		basic block Then
 * @param Bf		basic block False
 */
static void generate_conditional_quads(union astnode *expr, struct basic_block *Bt, struct basic_block *Bf);

#endif