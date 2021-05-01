/**
 * utilities related to (x86_64) target code genreation from quads
 */

#ifndef ASMGEN_H
#define ASMGEN_H

#include <quads/quads.h>
#include <stdlib.h>

// x86 opcodes
enum asm_opcode {
	AOC_PUSH,
	AOC_POP,
	AOC_MOV,
	AOC_LEAVE,
	AOC_ADD,
	AOC_SUB,
	AOC_MUL,
	AOC_DIV,
	AOC_CALL,
	AOC_RET,
    
};

// x86_64 instruction sizes
enum asm_size {
	AS_NONE,	// unspec
	AS_B,		// byte: size 1
	AS_W,		// word: size 2
	AS_L,		// long: size 4
	AS_Q,		// quad: size 8
};

// x86 asm directives/pseudo-opcodes
enum asm_pseudo_opcode {
	APOC_COMM,
	APOC_GLOBL,
	APOC_SIZE,

	APOC_TEXT,
	APOC_BSS,
	APOC_RODATA,
};

// x86 asm addressing modes
enum asm_addr_mode {
	AAM_REGISTER,	// %rax
	AAM_IMMEDIATE,	// $3
	AAM_MEMORY,	// a
	AAM_INDIRECT,	// (%rbp)
	AAM_REG_OFF,	// -4(%rbp)
};

/**
 * x86_64 registers
 * 
 * notes:
 * - scratch (caller-save) registers: A, C, D, DI, SI, 8-11
 * - long-term (callee-save) registers: B, SP, BP, 12-15
 * - not implenting fp registers (XMMx, STx)
 */
enum asm_reg_name {
	AR_A, AR_B, AR_C, AR_D, AR_DI, AR_SI, AR_BP, AR_SP,
	AR_8, AR_9, AR_10, AR_11, AR_12, AR_13, AR_14, AR_15
};

enum asm_component_type {
	ACT_INST,
	ACT_DIR,
	ACT_LABEL,
};

#define _ASM_COMPONENT \
	enum asm_component_type type; \
	union asm_component *next;

// x86 asm instruction
struct asm_inst {
	_ASM_COMPONENT

	enum asm_opcode oc;
	enum asm_size size;
	struct asm_addr *src, *dest;
};

// x86 asm directive/pseudo-opcode
struct asm_dir {
	_ASM_COMPONENT

	enum asm_pseudo_opcode poc;
	char *param1, *param2, *param3;
};

// x86 asm label
struct asm_label {
	_ASM_COMPONENT

	char *name;
};

// x86 register or sub-register
struct asm_reg {
	enum asm_reg_name name;
	enum asm_size size;
};

// x86 asm instruction src/dest operand
struct asm_addr {
	enum asm_addr_mode mode;
	union {
		struct asm_reg reg;
		// TODO: add more types here as necessary for the different
		// 	addressing modes
	} value;
};

/**
 * x86 language component; these will be chained together to form the (linear)
 * assembly code output
 */
union asm_component {
	struct asm_component_generic { _ASM_COMPONENT } generic;
	struct asm_inst inst;
	struct asm_dir dir;
	struct asm_label label;
};

#define ALLOC_AC(var, typ) \
	(var) = calloc(1, sizeof(union asm_component)); \
	(var)->generic.type = typ;

// linked list of instructions
extern union asm_component *asm_out;

// declare variables
void declare_symbol(union astnode *decl);

// dump string constant
void dump_string(union astnode *string);

// select x86-64 asm inst given quad
struct asm_inst *select_asm_inst(struct quad *quad, enum asm_size *size);

// select registers

// calling functions

// begin generating assembly from a basic_block list
void generate_asm(struct basic_block *bb_ll);

// print out assembly output
void print_asm();

#endif // asmgen.h