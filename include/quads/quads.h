/**
 * Defining the data structures and functions for quads and basic blocks.
 *
 * See also: exprquads.h for utilities specifically related to expression
 * quad generation.
 */

#ifndef QUADSH
#define QUADSH

#include <parser/astnode.h>

// current basic block being generated; easier to keep as a global variable
// because it's a pain to pass around during recursive control flow basic
// block generation;
// bb_ll is a linked list of the basic blocks in the desired order, which
// is controlled by adding basic blocks with bb_ll_push()
extern struct basic_block *cur_bb, *bb_ll;

/**
 * List of opcodes for the quad IR
 */
enum opcode {
	// pointer ops
	OC_LOAD,	// target = LOAD addr
	OC_STORE,	// STORE value, addr
	OC_LEA,		// target = LEA addr

	// usually we can avoid this with 3-address quads, but sometimes
	// we still need this (e.g., directly chained assignment)
	OC_MOV,		// target = MOV src

	// fncall; arglist is a linked list of addr values
	OC_CALL,	// target = CALL fn, arglist

	// arithmetic
	OC_ADD, OC_SUB, OC_MUL, OC_DIV, OC_MOD,

	// logical
	OC_LOGAND, OC_LOGOR, OC_LOGNOT,

	// bitwise
	OC_NOT, OC_AND, OC_OR, OC_XOR, OC_SHL, OC_SHR,

	// relational operators and condition codes
	// this mimics the x86 style where CMP sets condition codes,
	// which can return a variable (SETcc) or used for branches (JMPcc);
	// SETcc are generated by gen_lvalue();
	// JMPcc will be generated in target generation phase, currently
	// indicated by bb->branch_cc
	OC_CMP,		// CMP val1, val2
	OC_SETCC,	// target = SETCC type

	// generic cast operation -- may be noop, may not; exact implementation
	// is deferred to the target code generation stage; cast information
	// is stored in target and src type declaration info
	OC_CAST,	// target = CAST src

	OC_RET,		//return opcode
};

/**
 * condition codes -- closely related to the opcodes with similar names
 */
enum cc {
	// aliases: CC_ALWAYS used to indicate unconditional branch,
	// CC_UNSPEC when passing to gen_rvalue()
	CC_ALWAYS, CC_UNSPEC = 0,

	// regular condition codes
	CC_E, CC_NE, CC_L, CC_LE, CC_G, CC_GE
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
	// linked list (e.g., for fncall arglist)
	struct addr *next;

	// reference to astnode, temporary (pseudo-register), constant value,
	// string literal
	enum addr_type { AT_AST, AT_TMP, AT_CONST, AT_STRING } type;

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

	// memory offset for pseudo-registers (AT_TMP addr values) 
	// (this is due to us using a really simple (bad) register
	// "allocation" model -- every temporary value is memory-backed);
	// this has no meaning for non-tmp vals
	int offset;

	// astnode representation of addr type
	union astnode *decl;
};

/**
 * addressing mode for lvalues: AM_DIRECT for regular variables (memory values),
 * AM_INDIRECT for pointers (indirect memory values)
 */
enum addr_mode { AM_DIRECT, AM_INDIRECT };

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

	// next_def is default (fall-through) BB
	// next_cond is non-default (conditional) BB
	enum cc branch_cc;
	struct basic_block *next_def, *next_cond;

	// to store a regular linked list of basic blocks in order of
	// generation (for bb_ll); don't confuse this with next_def, next_cond
	struct basic_block *next;

	// indicate whether bb is finalized (i.e., quads reversed)
	// this gets set to 1 in link_bb(); allows us to detect when quads
	// are being generated in a defunct basic_block
	int finalized;
};

/**
 * generates a new basic block with a unique identifier
 *
 * this basic block may be added to the linearized linked-list of basic blocks
 * now, or it may be added after manually using the bb_ll_push function
 *
 * @param add_to_ll	whether to add the bb to the linearized ll now
 * @return		a new basic block
 */
struct basic_block *basic_block_new(int add_to_ll);

/**
 * returns the name of a basic block as a newly-allocated string, in the format
 * .BB.[fn name].[basic block id]
 * 
 * @param bb		basic block
 * @return		basic block name
 */
char *bb_name(struct basic_block *bb);

/**
 * adds the basic block to the linearized set of basic blocks. Calling this
 * manually allows you to set the position of a basic block in the linearization
 * of the basic blocks
 *
 * This MUST be called for every basic block we want to generate target code
 * for (i.e., not dummy basic blocks). Usually it will be called in
 * basic_block_new() if add_to_ll is set. If add_to_ll=0, then we delay the
 * insertion of this basic block into bb_ll, and bb_ll_push() MUST be called
 * manually.
 *
 * This is desirable in the case of certain control flow structures when we want
 * to put this basic block at a specific position to optimize the number of
 * jumps; see usage examples in cfquads.c
 *
 * @param bb 		basic block to add to the linearized set
 */
void bb_ll_push(struct basic_block *bb);

/**
 * generates a dummy basic block when we don't want to emit quads (e.g., in
 * sizeof operation). The dummy basic block will have ID=-1 and should not
 * be linked into the final CFG
 *
 * @return		a new dummy basic block
 */
struct basic_block *dummy_basic_block_new(void);

/**
 * emits a new quad to the specified basic block
 *
 * any of the operands or src may be null, depending on the opcode
 *
 * @param opcode	quad opcode
 * @param dest		quad destination; must be an lvalue
 * @param src1		quad first operand
 * @param src2		quad second operand
 * @return		generated quad
 */
struct quad *quad_new(enum opcode opcode, struct addr *dest, struct addr *src1,
	struct addr *src2);

/**
 * constructs and returns a new struct addr (operand/dest to quad)
 *
 * @param type		type of addr (memory (variable), immediate (constant),
 * 			or temporary (register))
 * @param decl		astnode representation of the type of the value
 * @return		constructed struct addr
 */
struct addr *addr_new(enum addr_type type, union astnode *decl);

/**
 * constructs and returns a new temporary pseudo-register (for subexpressions)
 *
 * also gives the struct addr a unique ID
 *
 * @param decl		astnode representation of the type of the value
 * @return		constructed struct addr
 */
struct addr *tmp_addr_new(union astnode *decl);

/**
 * recursively generate quads for a list of statements
 *
 * @param stmt		linked list of statements to generate quads for
 */
void gen_stmt_quads(union astnode *stmt);

/**
 * Generate basic blocks and quads for a function (top-level)
 *
 * Like complex declarations, basic blocks are built "in reverse" (by nature
 * of a singly-linked list) and then reversed when complete. While having a
 * second pointer would make it easy to build in the correct order, we do away
 * with all the troubles of maintaining extra pointers. (Alternatively, we
 * could append to the end of the list, but this would be less efficient
 * w/ O(N) rather than O(1) inserts). The bb will be reversed when it is
 * complete, i.e., when link_bbs is called on the basic block
 *
 * @param fn_decl		declarator for a function definition
 */
struct basic_block *generate_quads(union astnode *fn_decl);

#endif