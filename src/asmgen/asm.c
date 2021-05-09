#include <parser/scope.h>
#include <quads/sizeof.h>
#include <quads/exprquads.h>
#include <asmgen/asm.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// linearized linked list of directives
union asm_component *asm_out;

// x86_64 param register order; we assume no more than 6 parameters in a fncall
static enum asm_reg_name param_reg[] = {AR_DI, AR_SI, AR_D, AR_C, AR_8, AR_9};

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
		case AT_TMP:	asm_addr->mode = AAM_MEMORY;	break;
		case AT_CONST:	asm_addr->mode = AAM_IMMEDIATE;	break;
		case AT_STRING:	asm_addr->mode = AAM_IMMEDIATE;	break;
	}
	
	switch(addr->size)
	{
		case 1:	asm_addr->size = AS_B;	break;
		case 2:	asm_addr->size = AS_W;	break;
		case 4:	asm_addr->size = AS_L;	break;
		case 8:	asm_addr->size = AS_Q;	break;
		default:
			// this may happen if array; this is ok
			yyerror("addr2asmaddr: unknown size");
			asm_addr->size = AS_Q;
	}

	asm_addr->value.addr = addr;

	return asm_addr;
}

//select the opcodes and find the size 
struct asm_inst *select_asm_inst(struct quad *quad)
{
	union asm_component *cmp;
	struct asm_inst *inst;
	struct asm_addr *tmp1, *tmp2, *tmp3;
	struct asm_addr *src1, *src2, *dest;
	enum asm_size size_tmp;

	switch(quad->opcode)
	{
		case OC_MOV:
			// have to break this up in case of memory-memory mov
			// in quad opcode
			src1 = addr2asmaddr(quad->src1);
			dest = addr2asmaddr(quad->dest);

			tmp1 = reg2addr(AR_A, src1->size);

			cmp = asm_inst_new(AOC_MOV, src1, tmp1, src1->size);
			asm_inst_new(AOC_MOV, tmp1, dest, src1->size);

			ADD_COMMENT(cmp, "MOV");
			break;

		//arithmetic
    		case OC_ADD:
			src1 = addr2asmaddr(quad->src1);
			src2 = addr2asmaddr(quad->src2);
			dest = addr2asmaddr(quad->dest);
			size_tmp = MAX(src1->size, src2->size);
			tmp1 = reg2addr(AR_A, size_tmp);
			cmp = asm_inst_new(AOC_MOV, src1, tmp1, size_tmp);
			asm_inst_new(AOC_ADD, src2, tmp1, size_tmp);
			asm_inst_new(AOC_MOV, tmp1, dest, size_tmp);

			ADD_COMMENT(cmp, "ADD");
			break;

		case OC_SUB:
			src1 = addr2asmaddr(quad->src1);
			src2 = addr2asmaddr(quad->src2);
			dest = addr2asmaddr(quad->dest);
			size_tmp = MAX(src1->size, src2->size);
			tmp1 = reg2addr(AR_A, size_tmp);
			cmp = asm_inst_new(AOC_MOV, src1, tmp1, size_tmp);
			asm_inst_new(AOC_SUB, src2, tmp1, size_tmp);
			asm_inst_new(AOC_MOV, tmp1, dest, size_tmp);

			ADD_COMMENT(cmp, "SUB");
			break;

		case OC_MUL:
			src1 = addr2asmaddr(quad->src1);
			src2 = addr2asmaddr(quad->src2);
			dest = addr2asmaddr(quad->dest);
			size_tmp = MAX(src1->size, src2->size);
			tmp1 = reg2addr(AR_A, size_tmp);
			cmp = asm_inst_new(AOC_MOV, src1, tmp1, size_tmp);
			asm_inst_new(AOC_MUL, src2, tmp1, size_tmp);
			asm_inst_new(AOC_MOV, tmp1, dest, size_tmp);

			ADD_COMMENT(cmp, "MUL");
			break;

		case OC_DIV:;
			src1 = addr2asmaddr(quad->src1);
			src2 = addr2asmaddr(quad->src2);
			dest = addr2asmaddr(quad->dest);
			size_tmp = MAX(src1->size, src2->size);
			tmp1 = reg2addr(AR_A, size_tmp);
			tmp2 = reg2addr(AR_D, size_tmp);
			//push %rax
			cmp = asm_inst_new(AOC_PUSH, tmp1, NULL, AS_Q);
			//mov src1 to %rax
			asm_inst_new(AOC_MOV, src1, tmp1, AS_Q);
			//push %rdx
			asm_inst_new(AOC_PUSH, tmp2, NULL, AS_Q);
			//clear %rdx to all 0
			asm_inst_new(AOC_XOR, tmp2, tmp2, tmp2->size);
			//divide by divisor src2
			asm_inst_new(AOC_DIV, src2, NULL, size_tmp);
			//mov result from %eax
			asm_inst_new(AOC_MOV, tmp1, dest, size_tmp);

			ADD_COMMENT(cmp, "DIV");
			break;
			

		case OC_MOD:;
			src1 = addr2asmaddr(quad->src1);
			src2 = addr2asmaddr(quad->src2);
			dest = addr2asmaddr(quad->dest);
			size_tmp = MAX(src1->size, src2->size);
			tmp1 = reg2addr(AR_A, size_tmp);
			tmp2 = reg2addr(AR_D, size_tmp);
			//push %rax
			cmp = asm_inst_new(AOC_PUSH, tmp1, NULL, AS_Q);
			//mov src1 to %rax
			asm_inst_new(AOC_MOV, src1, tmp1, AS_Q);
			//push %rdx
			asm_inst_new(AOC_PUSH, tmp2, NULL, AS_Q);
			//clear %rdx to all 0
			asm_inst_new(AOC_XOR, tmp2, tmp2, tmp2->size);
			//divide by divisor src2
			asm_inst_new(AOC_DIV, src2, NULL, size_tmp);
			//mov result from %eax
			asm_inst_new(AOC_MOV, tmp2, dest, size_tmp);

			ADD_COMMENT(cmp, "MOD");
			break;

		

		case OC_LEA:;
			src1 = addr2asmaddr(quad->src1);
			dest = addr2asmaddr(quad->dest);
			tmp1 = reg2addr(AR_A, AS_Q);
			cmp = asm_inst_new(AOC_LEA, src1, tmp1, AS_Q);
			asm_inst_new(AOC_MOV, tmp1, dest, AS_Q);

			ADD_COMMENT(cmp, "LEA");
			break;
		
		case OC_LOAD:;
			src1 = addr2asmaddr(quad->src1);			
			dest = addr2asmaddr(quad->dest);

			tmp1 = reg2addr(AR_A, src1->size);
			tmp2 = reg2addr(AR_C, dest->size);
			tmp3 = reg2addr(AR_A, src1->size);
			tmp3->mode = AAM_INDIRECT;

			cmp = asm_inst_new(AOC_MOV, src1, tmp1, src1->size);
			asm_inst_new(AOC_MOV, dest, tmp2, dest->size);
			asm_inst_new(AOC_MOV, tmp3, tmp2, dest->size);
			asm_inst_new(AOC_MOV, tmp2, dest, dest->size);

			ADD_COMMENT(cmp, "LOAD");
			break;

		case OC_STORE:;
			src1 = addr2asmaddr(quad->src1);
			src2 = addr2asmaddr(quad->src2);
			tmp1 = reg2addr(AR_A, src1->size);
			tmp2 = reg2addr(AR_C, src2->size);
			tmp3 = reg2addr(AR_C, src2->size);
			tmp3->mode = AAM_INDIRECT;

			cmp = asm_inst_new(AOC_MOV, src1, tmp1, src1->size);
			asm_inst_new(AOC_MOV, src2, tmp2, src2->size);
			asm_inst_new(AOC_MOV, tmp1, tmp3, src1->size);

			ADD_COMMENT(cmp, "STORE");
			break;

		case OC_CALL:;
			// handle fncall arguments
			// param_reg
			struct addr *iter;
			int param_count = 0;
			_LL_FOR(quad->src2, iter, next) {
				if (param_count == 6) {
					yyerror("not supporting more than 6"
						" arguments in fncall");
					break;
				}

				src2 = addr2asmaddr(iter);
				asm_inst_new(AOC_MOV, src2,
					reg2addr(param_reg[param_count++],
					src2->size), src2->size);
			}

			// clear eax register (for non-vector argument list)
			asm_inst_new(AOC_XOR, reg2addr(AR_A, AS_L),
				reg2addr(AR_A, AS_L), AS_L);

			// emit call opcode
			struct asm_addr *addr_label = calloc(1,
				sizeof(struct asm_addr));
			addr_label->mode = AAM_LABEL;
			addr_label->size = AS_Q;
			addr_label->value.label
				= quad->src1->val.astnode->decl.ident;
			//printf("call_label:%s", addr_label->value.label);
			asm_inst_new(AOC_CALL, addr_label, NULL, AS_NONE);
			if (quad->dest){
				dest = addr2asmaddr(quad->dest);
				tmp1 = reg2addr(AR_A, dest->size);
				asm_inst_new(AOC_MOV, tmp1, dest, dest->size);
			}
			break;

		case OC_CMP:;
			src1 = addr2asmaddr(quad->src1);
			src2 = addr2asmaddr(quad->src2);
			tmp1 = reg2addr(AR_A, src1->size);
			asm_inst_new(AOC_MOV, src1, tmp1, src1->size);
			asm_inst_new(AOC_CMP, src2, tmp1, src2->size);
			break;
			
		case OC_SETCC:;
			dest = addr2asmaddr(quad->dest);
			enum asm_opcode oc;
			switch((enum cc) *quad->src1->val.constval){
				case CC_E:		oc = AOC_SETE; break;
				case CC_NE: 		oc = AOC_SETNE; break;
				case CC_L: 		oc = AOC_SETL; break;
				case CC_LE:		oc = AOC_SETLE; break;
				case CC_G: 		oc = AOC_SETG; break;
				case CC_GE:		oc = AOC_SETGE; break;
			}
			asm_inst_new(oc, dest, NULL, AS_NONE);

			break;

		case OC_RET:;
			src1 = addr2asmaddr(quad->src1);
			struct asm_addr *reg_ret = reg2addr(AR_A, src1->size);
			cmp = asm_inst_new(AOC_MOV, src1, reg_ret, src1->size);
			asm_inst_new(AOC_LEAVE, NULL, NULL, AS_NONE);
			asm_inst_new(AOC_RET, NULL, NULL, AS_NONE);
			
			ADD_COMMENT(cmp, "RETURN");
			break;
		
		case OC_CAST:
			src1 = addr2asmaddr(quad->src1);
			dest = addr2asmaddr(quad->dest);

			tmp1 = reg2addr(AR_A, src1->size);
			tmp2 = reg2addr(AR_A, dest->size);

			cmp = asm_inst_new(AOC_MOV, src1, tmp1, src1->size);

			// reinterpret cast; input and output are the same
			// size; noop

			// cast 4 byte -> 8 byte
			if (tmp1->size == 4 && tmp2->size == 8) {
				asm_inst_new(AOC_CLTQ, NULL, NULL, AS_NONE);
			}
			
			// cast 8 byte -> 4 byte
			// noop
			
			asm_inst_new(AOC_MOV, tmp2, dest, dest->size);

			ADD_COMMENT(cmp, "CAST");
			break;

		default:
			yyerror("select_asm_inst: unhandled opcode");
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
	union astnode *var_iter;
	union asm_component *component;
	struct basic_block *bb_iter;
	struct quad *quad_iter;
	char *fnname = strdup(fndecl->decl.ident),
		*tmp_fnname = malloc(strlen(fndecl->decl.ident) + 3);
	int offset = 0, param_count = 0;
	struct asm_addr *asm_addr;
	struct addr *addr;
	FILE *fp = stdout;

	// clear the current assembly
	asm_out = NULL;

	// GET FUNCTION LOCAL VARIABLES & PARAMETERS; assign offsets to
	// all variables
	_LL_FOR(fndecl->decl.fn_scope->symbols_ll, var_iter, decl.symbol_next) {
		offset -= astnode_sizeof_symbol(var_iter);
		var_iter->decl.offset = offset;

		// TODO: make sure these don't get printed out to the output
		// 	assembly file
		if (var_iter->decl.is_proto) {
			fprintf(fp, "proto");
			++param_count;
		} else {
			fprintf(fp, "local");
		}
		fprintf(fp, " var: %s (size: %d; offset: %d)\n",
			var_iter->decl.ident,
			astnode_sizeof_symbol(var_iter),
			var_iter->decl.offset);
	}

	// ALSO TREAT ALL PSEUDO-REGISTERS LIKE LOCAL VARS (give them a memory
	// address/offset on the stack)
	
	// helper for the following
	#define SET_ADDR_OF(addr)\
		if (quad_iter->addr && !quad_iter->addr->offset\
			&& quad_iter->addr->type == AT_TMP) {\
			offset -= quad_iter->addr->size;\
			quad_iter->addr->offset = offset;\
			\
			fprintf(fp, "tmp %d (size: %d; offset: %d)\n",\
				quad_iter->addr->val.tmpid,\
				quad_iter->addr->size,\
				quad_iter->addr->offset);\
		}

	_LL_FOR(bb_ll, bb_iter, next) {
		_LL_FOR(bb_iter->ll, quad_iter, next) {
			SET_ADDR_OF(dest);
			SET_ADDR_OF(src1);
			SET_ADDR_OF(src2);
		}
	}

	// FUNCTION PROLOGUE
	asm_dir_new(APOC_TEXT);
	asm_label_new(fndecl->decl.ident);
	
	asm_inst_new(AOC_PUSH, reg2addr(AR_BP, AS_Q), NULL, AS_Q);
	asm_inst_new(AOC_MOV, reg2addr(AR_SP, AS_Q), reg2addr(AR_BP, AS_Q),
		AS_Q);

	// allocate space on the stack for all the local variables
	// (this includes space for all of the temporary values)
	addr = addr_new(AT_CONST, create_size_t());
	*((uint64_t*)addr->val.constval) = -offset;
	asm_inst_new(AOC_SUB, addr2asmaddr(addr), reg2addr(AR_SP, AS_Q),
		AS_Q);

	// copy all parameters into memory locations
	if (param_count > 6) {
		yyerror("not supporting more than 6 function parameters");
	}
	_LL_FOR(fndecl->decl.fn_scope->symbols_ll, var_iter, decl.symbol_next) {
		if (!param_count) {
			break;
		}

		if (var_iter->decl.is_proto) {
			addr = addr_new(AT_AST, var_iter->decl.components);
			addr->val.astnode = var_iter;
			asm_addr = addr2asmaddr(addr);
			asm_inst_new(AOC_MOV, reg2addr(param_reg[--param_count],
				asm_addr->size), asm_addr, asm_addr->size);
		}
	}

	// FUNCTION BODY
	_LL_FOR(bb_ll, bb_iter, next) {
		asm_label_new(bb_name(bb_iter));

		_LL_FOR(bb_iter->ll, quad_iter, next) {
			select_asm_inst(quad_iter);
		}
		print_CC(bb_iter);
	}

	// GENERATE EPILOGUE
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


// print controls and branches
void print_CC(struct basic_block *bb){
	FILE *fp = stdout;
	struct asm_addr *addr_label1, *addr_label2;
	enum asm_opcode oc;

	if (bb->next_cond){
		addr_label1 = calloc(1, sizeof(struct asm_addr));
		addr_label1->mode = AAM_LABEL;
		addr_label1->size = AS_Q;	
		addr_label1->value.label = bb_name(bb->next_cond);
		switch(bb->branch_cc){
			case CC_E:	oc = AOC_JE; break;
			case CC_NE:	oc = AOC_JNE; break;
			case CC_L:	oc = AOC_JL; break;
			case CC_LE:	oc = AOC_JLE; break;
			case CC_G:	oc = AOC_JG; break;
			case CC_GE:	oc = AOC_JGE; break;
		}
		asm_inst_new(oc, addr_label1, NULL, AS_NONE);
	}

	if (bb->next_def) {
		addr_label2 = calloc(1, sizeof(struct asm_addr));
		addr_label2->mode = AAM_LABEL;
		addr_label2->size = AS_Q;
		addr_label2->value.label = bb_name(bb->next_def);
		asm_inst_new(AOC_JMP, addr_label2, NULL, AS_NONE);
	}
}


// print comment of asm_component, if applicable
void print_comment(char *comment) {
	FILE *fp = stdout;
	
	if (!comment) {
		return;
	}

	fprintf(fp, "\t\t#%s", comment);
}

// TODO: remove this predeclaration
void print_asm_dir(struct asm_dir *dir);

void print_asm_addr(struct asm_addr *addr)
{
	FILE *fp = stdout;
	struct asm_reg *reg;
	char *r1, *r2, *r3;
	unsigned char *const_val;
	union asm_component *dir;
	struct addr *quad_addr;
	union astnode *decl, *sc;
	int offset;

	switch (addr->mode) {
	case AAM_INDIRECT:
	case AAM_REGISTER:
		reg = &addr->value.reg;

		if (addr->mode == AAM_INDIRECT) {
			fprintf(fp, "(");
		}

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

		if (addr->mode == AAM_INDIRECT) {
			fprintf(fp, ")");
		}

		break;

	case AAM_MEMORY:
		quad_addr = addr->value.addr;

		if (quad_addr->type == AT_TMP) {
			fprintf(fp, "%d(%%rbp)", quad_addr->offset);
			break;
		}

		decl = quad_addr->val.astnode;
		if (!decl->decl.is_string) {
			sc = decl->decl.declspec->declspec.sc;
		}

		// if local (not extern or static) variable or pseudo-register:
		// use rbp-relative addressing
		// MOVL	$2, -4(%rbp)
		if (!decl->decl.is_string
			&& sc->sc.scspec != SC_EXTERN
			&& sc->sc.scspec != SC_STATIC) {
			fprintf(fp, "%d(%%rbp)", decl->decl.offset);
		}

		// if global (extern or static) variable:
		// use rip-relative addressing
		// MOVL	$2, i(%rip)
		else {
			fprintf(fp, "%s(%%rip)",
				!decl->decl.is_string
					&& sc->sc.scspec == SC_STATIC
				? decl->decl.static_uid
				: decl->decl.ident);
		}
		break;

	case AAM_LABEL:
		//NYI("printing label mode asm addr")
		fprintf(fp, "%s", addr->value.label);
		break;

	case AAM_REG_OFF:
		NYI("printing register-offset mode asm addr");
		break;

	case AAM_IMMEDIATE:
		const_val = addr->value.addr->val.constval;
		fprintf(fp, "$%llu", *((uint64_t*)const_val));
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
	case AOC_JL:	inst_text = "jl"; break;
	case AOC_JLE:	inst_text = "jle"; break;
	case AOC_JG:	inst_text = "jg"; break;
	case AOC_JGE:	inst_text = "jge"; break;
	case AOC_XOR:	inst_text = "xor"; break;
	case AOC_LEA:	inst_text = "lea"; break;
	case AOC_SETE:	inst_text = "sete"; break;
	case AOC_SETNE:	inst_text = "setne"; break;
	case AOC_SETL:	inst_text = "setl"; break;
	case AOC_SETLE:	inst_text = "setle"; break;
	case AOC_SETG:	inst_text = "setg"; break;
	case AOC_SETGE:	inst_text = "setge"; break;
	case AOC_CLTQ:	inst_text = "cltq"; break;
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
		fprintf(fp, "\t");
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
	case APOC_LOCAL:	dir_text = "local"; break;
	case APOC_SIZE:		dir_text = "size"; break;
	case APOC_TYPE:		dir_text = "type"; break;
	case APOC_TEXT:		dir_text = "text"; break;
	case APOC_BSS:		dir_text = "bss"; break;
	case APOC_SECTION:	dir_text = "section"; break;
	case APOC_STRING:	dir_text = "string"; break;
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
	union astnode *iter, *iter2;
	union asm_component *dir, *dir2;
	char size_buf[10];
	unsigned i;

	// clear current asm code
	asm_out = NULL;

	// dump global (extern/static) vars
	_LL_FOR(globals, iter, decl.symbol_next) {
		sprintf(size_buf, "%d", astnode_sizeof_symbol(iter));

		dir = asm_dir_new(APOC_COMM);
		if (iter->decl.static_uid) {
			dir->dir.param1 = iter->decl.static_uid;

			// also emit .local
			dir2 = asm_dir_new(APOC_LOCAL);
			dir2->dir.param1 = iter->decl.static_uid;
		} else {
			dir->dir.param1 = iter->decl.ident;

			// also emit .globl
			dir2 = asm_dir_new(APOC_GLOBL);
			dir2->dir.param1 = iter->decl.ident;
		}
		dir->dir.param2 = strdup(size_buf);

		fprintf(fp, "Got global variable: %s\n", iter->decl.static_uid
			? iter->decl.static_uid : iter->decl.ident);
	}

	_LL_FOR(string_ll, iter, decl.symbol_next) {
		dir = asm_dir_new(APOC_STRING);

		char *str = malloc(4096);
		sprintf(str, "\"%s\"", print_string(&iter->string.string));
		dir->dir.param1 = str;

		asm_label_new(iter->string.label);
	}

	dir = asm_dir_new(APOC_SECTION);
	dir->dir.param1 = ".rodata";

	print_asm();
}