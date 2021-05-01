#include <asmgen/asm.h>
#include <stdio.h>
#include <string.h>

union asm_component *asm_out;

// generate asm instruction, add to ll
union asm_component *asm_inst_new(enum asm_opcode oc, struct asm_addr *src,
	struct asm_addr *dest, enum asm_size size)
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
	return component;
}

// generate an asm directive, add to ll
// have to add arguments manually
union asm_component *asm_dir_new(enum asm_pseudo_opcode poc)
{
	union asm_component *component;

	ALLOC_AC(component, ACT_DIR);

	component->dir.poc = poc;

	// insert instruction into ll
	component->generic.next = asm_out;
	asm_out = component;
	return component;
}

// generate an asm directive, add to ll
// have to add arguments manually
union asm_component *asm_label_new(char *name)
{
	union asm_component *component;

	ALLOC_AC(component, ACT_LABEL);

	component->label.name = strdup(name);

	// insert instruction into ll
	component->generic.next = asm_out;
	asm_out = component;
	return component;
}

struct asm_addr *reg2addr(enum asm_reg_name name, enum asm_size size)
{
	struct asm_addr *addr = calloc(1, sizeof(struct asm_addr));

	addr->mode = AAM_REGISTER;
	addr->size = size;
	addr->value.reg.name = name;
	addr->value.reg.size = size;

	return addr;
}

struct asm_addr *addr2asmaddr(struct addr *addr)
{
	struct asm_addr *asm_addr = calloc(1, sizeof(struct asm_addr));
	switch(addr->type)
	{
		case AT_AST:	asm_addr->mode = AAM_MEMORY;	break;
		case AT_TMP:	asm_addr->mode = AAM_REGISTER;	break;
		case AT_CONST:	asm_addr->mode = AAM_IMMEDIATE;	break;
		case AT_STRING:	asm_addr->mode = AAM_IMMEDIATE;	break;
	}
	
	switch(addr->size)
	{
		case 1:	asm_addr->size = AS_B;	break;
		case 2:	asm_addr->size = AS_W;	break;
		case 4:	asm_addr->size = AS_L;	break;
		case 8:	asm_addr->size = AS_Q;	break;
	}
	
	asm_addr->value.addr = *addr;
	
	return asm_addr;


}

#define MAX(a, b) ((a) > (b) ? (a) : (b))

//select the opcodes and find the size 
struct asm_inst *select_asm_inst(struct quad *quad, enum asm_size *size)
{
	struct asm_inst *inst;
	struct asm_addr *tmp1, *tmp2, *tmp3;
	struct asm_addr *src1, *src2, *dest;
	enum asm_size size_tmp;

	switch(quad->opcode)
	{
		case OC_MOV:;
			src1 = addr2asmaddr(quad->src1);
			dest = addr2asmaddr(quad->dest);
			asm_inst_new(AOC_MOV, src1, dest, src1->size);
			break;

		//arithmetic
    	case OC_ADD:
			src1 = addr2asmaddr(quad->src1);
			src2 = addr2asmaddr(quad->src2);
			dest = addr2asmaddr(quad->dest);
			size_tmp = MAX(src1->size, src2->size);
			tmp1 = reg2addr(AR_A, size_tmp);
			asm_inst_new(AOC_MOV, src1, tmp1, size_tmp);
			asm_inst_new(AOC_ADD, src2, tmp1, size_tmp);
			asm_inst_new(AOC_MOV, tmp1, dest, size_tmp);
			break;

		case OC_SUB:
			src1 = addr2asmaddr(quad->src1);
			src2 = addr2asmaddr(quad->src2);
			dest = addr2asmaddr(quad->dest);
			size_tmp = MAX(src1->size, src2->size);
			tmp1 = reg2addr(AR_A, size_tmp);
			asm_inst_new(AOC_MOV, src1, tmp1, size_tmp);
			asm_inst_new(AOC_SUB, src2, tmp1, size_tmp);
			asm_inst_new(AOC_MOV, tmp1, dest, size_tmp);
			break;

		case OC_MUL:
			src1 = addr2asmaddr(quad->src1);
			src2 = addr2asmaddr(quad->src2);
			dest = addr2asmaddr(quad->dest);
			size_tmp = MAX(src1->size, src2->size);
			tmp1 = reg2addr(AR_A, size_tmp);
			asm_inst_new(AOC_MOV, src1, tmp1, size_tmp);
			asm_inst_new(AOC_MUL, src2, tmp1, size_tmp);
			asm_inst_new(AOC_MOV, tmp1, dest, size_tmp);
			break;

		case OC_DIV:
			break;
			

		case OC_MOD:
			break;

		
		// case OC_LEA:

		// case OC_CALL:

		case OC_CMP:
			break;
			
		case OC_SETCC:
			break;

		case OC_RET:;
			struct asm_addr *reg_ret = reg2addr(AR_A, AS_Q);
			asm_inst_new(AOC_MOV, addr2asmaddr(quad->src1), reg_ret, AS_NONE);
			asm_inst_new(AOC_LEAVE, NULL, NULL, AS_NONE);
			asm_inst_new(AOC_RET, NULL, NULL, AS_NONE);
			break;
	}
}

/**
 * reverse order of asm_out
 * 
 * Same algorithm used to reverse ll for declarator chains, quads, and bbs
 */
static void reverse_asm_components(void)
{
	union asm_component *a, *b, *c;

	if (!(a = asm_out) || !(b = LL_NEXT(a))) {
		return;
	}

	c = LL_NEXT(b);
	LL_NEXT(a) = NULL;

	while (c) {
		LL_NEXT(b) = a;
		a = b;
		b = c;
		c = LL_NEXT(c);
	}

	LL_NEXT(b) = a;
	asm_out = b;
}

/**
 * general function layout
 * 
 * 	.text	# .just to be safe
 * fnname:
 * 	pushq	%rbp		
 * 	movq	%rsp, %rbp
 * 	subq	%rsp, $size_of_localvars
 * 
 * 	# fn body
 * 
 * 	leave
 * 	ret
 * 
 * 	.size fnname .-$
 * 	.globl fnname
 * 	.type fnname, @function
 */
void generate_asm(union astnode *fndecl, struct basic_block *bb_ll)
{
	union asm_component *component;
	char *fnname = strdup(fndecl->decl.ident),
		*tmp_fnname = malloc(strlen(fndecl->decl.ident) + 3);

	// clear the current assembly
	asm_out = NULL;

	// generate prologue
	asm_dir_new(APOC_TEXT);
	asm_label_new(fndecl->decl.ident);
	
	asm_inst_new(AOC_PUSH, reg2addr(AR_BP, AS_Q), NULL, AS_Q);
	asm_inst_new(AOC_MOV, reg2addr(AR_SP, AS_Q), reg2addr(AR_BP, AS_Q),
		AS_Q);

	// TODO: make second operand the size of all the local variables
	asm_inst_new(AOC_SUB, reg2addr(AR_SP, AS_Q), reg2addr(AR_SP, AS_Q),
		AS_Q);

	// generate epilogue
	asm_inst_new(AOC_LEAVE, NULL, NULL, AS_NONE);
	asm_inst_new(AOC_RET, NULL, NULL, AS_NONE);

	component = asm_dir_new(APOC_SIZE);
	strcpy(tmp_fnname, ".-");
	strcat(tmp_fnname, fnname);
	component->dir.param1 = fnname;
	component->dir.param2 = tmp_fnname;

	component = asm_dir_new(APOC_GLOBL);
	component->dir.param1 = fnname;

	component = asm_dir_new(APOC_TYPE);
	component->dir.param1 = fnname;
	component->dir.param2 = "@function";

	// reverse asm components
	reverse_asm_components();

	// print out the asm for this function
	print_asm();
}

// print comment of asm_component, if applicable
void print_comment(char *comment) {
	FILE *fp = stdout;
	
	if (!comment) {
		return;
	}

	fprintf(fp, "\t#%s", comment);
}

void print_asm_addr(struct asm_addr *addr)
{
	// TODO: switch on the addressing modes
	FILE *fp = stdout;
	struct asm_reg *reg;
	char *r1, *r2, *r3;

	switch (addr->mode) {
	case AAM_REGISTER:
		reg = &addr->value.reg;

		r1 = r3 = "";

		switch (reg->name) {
		case AR_A:	r2 = "a"; break;
		case AR_B:	r2 = "b"; break;
		case AR_C:	r2 = "c"; break;
		case AR_D:	r2 = "d"; break;
		case AR_DI:	r2 = "di"; break;
		case AR_SI:	r2 = "si"; break;
		case AR_BP:	r2 = "bp"; break;
		case AR_SP:	r2 = "sp"; break;
		case AR_8:	r2 = "8"; break;
		case AR_9:	r2 = "9"; break;
		case AR_10:	r2 = "10"; break;
		case AR_11:	r2 = "11"; break;
		case AR_12:	r2 = "12"; break;
		case AR_13:	r2 = "13"; break;
		case AR_14:	r2 = "14"; break;
		case AR_15:	r2 = "15"; break;
		}

		// old registers, e.g., rax, eax, ax, al
		if (reg->name < AR_8) {
			switch (reg->size) {
			case AS_B:	r3 = "l"; break;
			case AS_W:	break;
			case AS_L:	r1 = "e"; break;
			case AS_Q:	r1 = "r"; break;
			case AS_NONE:
				yyerror_fatal("must specify register size");
			}
			if (reg->name < AR_DI && reg->size != AS_B) {
				r3 = "x";
			}
			
		}
		// new registers, e.g., r8, r8b, r8w, r8d
		else {
			r1 = "r";
			switch (reg->size) {
			case AS_B:	r3 = "b"; break;
			case AS_W:	r3 = "w"; break;
			case AS_L:	r3 = "d"; break;
			case AS_Q:	break;
			case AS_NONE:
				yyerror_fatal("must specify register size");
			}
		}

		fprintf(fp, "%%%s%s%s", r1, r2, r3);

		break;
	case AAM_MEMORY:
		NYI("printing direct (memory) mode asm addr ");
		break;
	case AAM_INDIRECT:
		NYI("printing indirect mode asm addr");
		break;
	case AAM_REG_OFF:
		NYI("printing register-offset mode asm addr");
		break;
	default:
		yyerror_fatal("unknown asm addressing mode");
	}
}

void print_asm_inst(struct asm_inst *inst)
{
	FILE *fp = stdout;
	char *inst_text, *size_text;

	switch (inst->oc) {
	case AOC_PUSH:	inst_text = "push"; break;
	case AOC_POP:	inst_text = "pop"; break;
	case AOC_MOV:	inst_text = "mov"; break;
	case AOC_LEAVE:	inst_text = "leave"; break;
	case AOC_ADD:	inst_text = "add"; break;
	case AOC_SUB:	inst_text = "sub"; break;
	case AOC_MUL:	inst_text = "imul"; break;
	case AOC_DIV:	inst_text = "idiv"; break;
	case AOC_CALL:	inst_text = "call"; break;
	case AOC_RET:	inst_text = "ret"; break;
	case AOC_CMP:	inst_text = "cmp"; break;
	case AOC_JMP:	inst_text = "jmp"; break;
	case AOC_JE:	inst_text = "je"; break;
	case AOC_JNE:	inst_text = "jne"; break;
	}

	switch (inst->size) {
	case AS_NONE:	size_text = ""; break;
	case AS_B:	size_text = "b"; break;
	case AS_W:	size_text = "w"; break;
	case AS_L:	size_text = "l"; break;
	case AS_Q:	size_text = "q"; break;
	default:
		yyerror_fatal("unknown asm instruction size");
	}

	fprintf(fp, "\t%s%s", inst_text, size_text);

	if (inst->src) {
		fprintf(fp, " ");
		print_asm_addr(inst->src);

		if (inst->dest) {
			fprintf(fp, ", ");
			print_asm_addr(inst->dest);
		}
	}

	print_comment(inst->comment);
	fprintf(fp, "\n");
}

void print_asm_dir(struct asm_dir *dir)
{
	FILE *fp = stdout;
	char *dir_text;

	switch (dir->poc) {
	case APOC_COMM:		dir_text = "comm"; break;
	case APOC_GLOBL:	dir_text = "globl"; break;
	case APOC_SIZE:		dir_text = "size"; break;
	case APOC_TYPE:		dir_text = "type"; break;
	case APOC_TEXT:		dir_text = "text"; break;
	case APOC_BSS:		dir_text = "bss"; break;
	case APOC_RODATA:	dir_text = "rodata"; break;
	default:
		yyerror_fatal("unknown asm directive");
	}

	fprintf(fp, "\t.%s", dir_text);

	// print arguments if they exist
	if (dir->param1) {
		fprintf(fp, " %s", dir->param1);

		if (dir->param2) {
			fprintf(fp, ", %s", dir->param2);

			if (dir->param3) {
				fprintf(fp, ", %s", dir->param3);
			}
		}
	}

	print_comment(dir->comment);
	fprintf(fp, "\n");
}

void print_asm_label(struct asm_label *label)
{
	FILE *fp = stdout;

	
	fprintf(fp, "%s:", label->name);
	print_comment(label->comment);
	fprintf(fp, "\n");
}

void print_asm()
{
	FILE *fp = stdout;
	union asm_component *iter;

	LL_FOR(asm_out, iter) {
		switch (iter->generic.type) {
		case ACT_INST:
			print_asm_inst(&iter->inst);
			break;
		case ACT_DIR:
			print_asm_dir(&iter->dir);
			break;
		case ACT_LABEL:
			print_asm_label(&iter->label);
			break;
		}
	}
}

void gen_globalvar_asm(union astnode *globals)
{
	FILE *fp = stdout;
	union astnode *iter;

	// clear current asm code
	asm_out = NULL;

	_LL_FOR(globals, iter, decl.global_next) {
		// TODO: emit instructions for all these global variables
	}
}