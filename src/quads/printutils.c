#include <lexer/stringutils.h>
#include <parser/astnode.h>
#include <parser/scope.h>
#include <quads/printutils.h>
#include <stdio.h>
#include <stdint.h>

char *opcode2str(enum opcode oc)
{
	switch (oc) {
	case OC_LOAD:	return "LOAD";
	case OC_STORE:	return "STORE";
	case OC_LEA:	return "LEA";
	case OC_ADD:	return "ADD";
	case OC_SUB:	return "SUB";
	case OC_MUL:	return "MUL";
	case OC_DIV:	return "DIV";
	case OC_MOV:	return "MOV";
	case OC_CMP:	return "CMP";
	case OC_CALL:	return "CALL";
	case OC_RET:	return "RETURN";
	case OC_SETCC:	return "SETCC";

	// pseudo-opcode
	case OC_CAST:	return "CAST";
	}

	yyerror_fatal("invalid opcode");
}

char *cc2str(enum cc cc) {
	switch (cc) {
	case CC_ALWAYS:	return NULL;
	case CC_E: 	return "E";
	case CC_NE: 	return "NE";
	case CC_L:	return "L";
	case CC_LE:	return "LE";
	case CC_G:	return "G";
	case CC_GE:	return "GE";
	}
}

void print_addr(struct addr *addr)
{
	unsigned char *constval;
	enum scope_type st;
	union astnode *decl;

	if (!addr) {
		yyerror_fatal("quadgen: addr should not be NULL"
			" in print_addr()");
		return;
	}

	// indicate size of addr
	fprintf(dfp, "[%d:", addr->size);

#if DEBUG2
	// more intense debugging: print all struct addr types
	// probably unnecessary unless trying to debug something
	print_declarator(addr->decl, 0);
	fprintf(dfp, ":");
#endif

	switch (addr->type) {

	// print constant (scalar) value (as hex)
	case AT_CONST:
		constval = addr->val.constval;
		fprintf(dfp, "const:0x");
		switch (addr->size) {
		case 1: fprintf(dfp, "%x",  *((uint8_t *) constval)); break;
		case 2: fprintf(dfp, "%x",  *((uint16_t *)constval)); break;
		case 4: fprintf(dfp, "%x",  *((uint32_t *)constval)); break;
		case 8: fprintf(dfp, "%lx", *((uint64_t *)constval)); break;
		case 16: fprintf(dfp, "%lx%lx", *((uint64_t *)constval),
			*((uint64_t *)constval+1)); break;
		default: yyerror_fatal("quadgen: invalid size");
		}
		fprintf(dfp, "]");
		break;

	// print temporary value (id)
	case AT_TMP:
		fprintf(dfp, "tmp:%%%d]", addr->val.tmpid);
		break;

	// print symbol table value
	case AT_AST:
		decl = addr->val.astnode;

		if (decl->decl.is_implicit) {
			st = -1;
		} else if (!decl->decl.is_string) {
			// get scope; proto scopes are fully promoted to function scopes
			// so we have to check the is_proto member to see if a local
			// var is from the prototype or not
			st = decl->decl.is_proto ? ST_PROTO
				: decl->decl.scope->type;
		}

		fprintf(dfp, "%s:%s]",
			st == -1 ? "implicit" :
			st == ST_FILE ? "globalvar" :
			st == ST_PROTO ? "protovar" : "localvar",
			decl->decl.ident);
		break;

	case AT_STRING:
		fprintf(dfp, "string:\"%s\"]",
	  		print_string(&addr->val.astnode->string.string));
		break;

	default:
		yyerror_fatal("quadgen: unknown address type");
	}
}

void print_quad(struct quad *quad)
{
	struct addr *iter;

	if (!quad) {
		yyerror_fatal("quadgen: quad should not be NULL"
			" in print_quad()");
		return;
	}

	fprintf(dfp, "\t");

	// print dest addr (if applicable)
	if (quad->dest) {
		print_addr(quad->dest);
		fprintf(dfp, "=");
	}

	// print opcode
	fprintf(dfp, "%s ", opcode2str(quad->opcode));

	// print source addr (if applicable)
	// (0- and 1-operand opcodes exist)
	if (quad->src1) {
		print_addr(quad->src1);
		
		if (quad->src2) {
			fprintf(dfp, ", ");

			// regular opcodes
			if (quad->opcode != OC_CALL) {
				print_addr(quad->src2);
			}
			// fncall opcode: ll of fncall arglist
			else {
				fprintf(dfp, "(arglist");
				_LL_FOR(quad->src2, iter, next) {
					fprintf(dfp, " ");
					print_addr(iter);
				}
				fprintf(dfp, ")");
			}
		}
	}
	
	fprintf(dfp, "\n");
}

void print_basic_block(struct basic_block *bb)
{
	struct quad *iter;
	struct bb_history *hist_iter, *cur;

	// shouldn't happen
	if (!bb) {
		yyerror_fatal("quadgen: null bb in print_basic_block");
		return;
	}

	// print bb identifier
	fprintf(dfp, ".BB.%s.%d {\n", bb->fn_name, bb->bb_no);

	// print all quads in bb
	_LL_FOR(bb->ll, iter, next) {
		print_quad(iter);
	}

	// print what it branches to
	// conditional branch (if exists)
	if (bb->next_cond && bb->branch_cc != CC_ALWAYS) {
		fprintf(dfp, "\tJMP%s .BB.%s.%d\n", cc2str(bb->branch_cc),
			bb->next_cond->fn_name, bb->next_cond->bb_no);
	}
	// unconditional branch (if exists; doesn't exist for final
	// bb in function, i.e., CFG terminal node)
	if (bb->next_def) {
		fprintf(dfp, "\tJMP .BB.%s.%d\n",
			bb->next_def->fn_name, bb->next_def->bb_no);
	} else {
		fprintf(dfp, "\t(CFG terminal node)\n");
	}

	fprintf(dfp, "}\n");
}

void print_basic_blocks()
{
	struct basic_block *iter;

	_LL_FOR(bb_ll, iter, next) {
		print_basic_block(iter);
	}
}
