#include <asmgen/asm.h>
#include <stdlib.h>

union asm_component *asm_out;

// generate asm instruction, add to ll
void inst_new(enum asm_opcode oc, struct asm_addr *dest,
	struct asm_addr *src, enum asm_size size)
{
	union asm_component *component;

	ALLOC_AC(component, ACT_INST);

	component->inst.oc = oc;
	component->inst.dest = dest;
	component->inst.src = src;
	component->inst.size = size;

	// insert instruction into ll
	component->generic.next = asm_out;
	asm_out = component;
}

//select the opcodes and find the size 
struct asm_inst *select_asm_inst(struct quad *quad, enum asm_size *size)
{
	struct asm_inst *inst;
	switch(quad->opcode)
	{
		//arithmetic
    	// case OC_ADD:	
		// case OC_SUB: 
		// case OC_MUL: 
		// case OC_DIV: 

		// case OC_MOD:
		
		// case OC_LEA:

		// case OC_CALL:

		// case OC_RET:
		
	}

}




void generate_asm(struct basic_block *bb_ll)
{
	// generate prologue (pushq %rbp;movq %rsp,%rbp;)
	// also subq %rsp, (size of locals), assign local variables an offset
	// from %rsp
	// TODO: working here

	// generate epilogue (leave;ret)
	inst_new(AOC_LEAVE, NULL, NULL, AS_NONE);
	inst_new(AOC_RET, NULL, NULL, AS_NONE);
}

void print_asm()
{

}