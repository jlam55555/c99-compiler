/**
 *	This file includes utilities for typing that are too long to fit
 *	comfortably in a macro.
 */

#include <astnode.h>
#include <asttypes.h>
#include <scope.h>
#include <parser.h>
#include <decl.h>
#include <lexerutils/errorutils.h>
#include <stdio.h>

// declare constant ellipsis for use; this is declared in asttypes.c
//union astnode ELLIPSIS_DECLARATOR;

// for LL_APPEND to work
//union astnode *ll_append_iter;

char *print_scope(enum scope_type st);
void print_typequallist(union astnode *node);
char *print_sc(union astnode *scspec_node);
void fill_defaults(union astnode *declspec);
//void print_symbol(union astnode *node, int depth);

//// for printing out struct/union when it is defined
//void print_structunion_def(union astnode *node)
//{
//	FILE *outfile = stdout;
//	union astnode *iter;
//	struct astnode_typespec_structunion *su;
//
//	su = &node->ts_structunion;
//
//	fprintf(outfile, "struct %s definition at %s:%d{\n",
//		su->ident, su->def_filename, su->def_lineno);
//
//	// loop through fields
//	iter = su->members;
//	while (iter) {
//		//print_symbol(iter, 0);
//		iter = iter->generic.next;
//	}
//
//	fprintf(outfile, "}\n");
//}

// for printing NT_TS_* types (when declaring variables)
void print_type(union astnode *node, int depth)
{
	FILE *outfile = stdout;

	if (!node) {
		fprintf(outfile, "unspecified type\n");
		return;
	}

	switch (node->generic.type) {

	// scalar types
	case NT_TS_SCALAR:
		// signedness (for applicable types)
		if(node->ts_scalar.modifiers.sign==SIGN_UNSIGNED)
			fprintf(outfile, "unsigned");
			
		// long/long long/short (for applicable types)
		switch(node->ts_scalar.modifiers.lls) {
		case LLS_UNSPEC: break;
		case LLS_SHORT: fprintf(outfile, "short "); break;
		case LLS_LONG: fprintf(outfile, "long "); break;
		case LLS_LONG_LONG: fprintf(outfile, "long long "); break;
		}

		// "base" scalar type
		switch(node->ts_scalar.basetype) {
		case BT_UNSPEC: fprintf(outfile, "\n"); break;
		case BT_INT:
			if(node->ts_scalar.modifiers.lls == LLS_UNSPEC)
				fprintf(outfile, "int\n");
			else
				fprintf(outfile, "\n");
			break;
		case BT_FLOAT:
			fprintf(outfile, "float\n"); break; case BT_DOUBLE: fprintf(outfile, "double\n"); break;
		case BT_CHAR: fprintf(outfile, "char\n"); break;
		case BT_BOOL: fprintf(outfile, "bool\n"); break;
		}
		break;

	// struct types: only need to print tag and where it was defined
	case NT_TS_STRUCT_UNION:;
		struct astnode_typespec_structunion *su = &node->ts_structunion;
		if (su->is_complete) {
			fprintf(outfile, "struct %s (defined at %s:%d)\n",
				su->ident, su->def_filename, su->def_lineno);
		} else {
			fprintf(outfile, "struct %s (incomplete)\n",
				su->ident);
		}
		break;
	}
}

// recursive declarator print function
//void print_declarator(union astnode *node, int depth)
//{
//	FILE *outfile = stdout;
//
//	// TODO: fix later
//	if (!node) {
//		return;
//	}
//	switch(node->generic.type)
//	{
//		// direct declarator: handling arrays, fns
//		case NT_DIRDECLARATOR:
//			if (node->dirdeclarator.ident->generic.type != NT_IDENT) {
//				print_symbol(node->dirdeclarator.ident, depth+1);
//			}
//
//			switch(node->dirdeclarator.declarator_type)
//			{
//				case DT_REGULAR:
//
//					break;
//
//				case DT_ARRAY:
//					fprintf(outfile, "array of %d elements of type:\n", node->dirdeclarator.size->num.num.int_val);
//
//					// print_symbol(node->dirdeclarator.typequallist, depth+2);
//					break;
//
//				case DT_FN:
//					fprintf(outfile, "function returning\n");
//					if(node->dirdeclarator.paramtypelist)
//					{
//						fprintf(outfile, "and taking the following arguments\n");
//						print_type(node->dirdeclarator.paramtypelist, depth+1);
//					}
//					else
//						fprintf(outfile, "and taking an unspecified number of arguments.\n");
//					break;
//			}
//
//			break;
//
//		// handling (potentially nested) declarators
//		case NT_DECLARATOR:;
//			struct astnode_declarator *declarator = &node->declarator;
//
//			print_symbol(declarator->dirdeclarator, depth+1);
//
//			// pointers
//			if(declarator->pointer) {
//				print_symbol(declarator->pointer, depth+1);
//			}
//			break;
//
//		// pointer included in declarator
//		case NT_POINTER:
//			print_typequallist(node->ptr.typequallist);
//			fprintf(outfile, "pointer to\n");
//			print_symbol(node->ptr.to, depth+2);
//			break;
//
//		case NT_SYMBOL:;
//			struct scope *curscope = get_current_scope();
//			fprintf(outfile, "%s is defined at %s:%d [in %s scope starting at %s:%d] as a ",
//			 	node->symbol.ident, filename, lineno, print_scope(curscope->type), curscope->filename, curscope->lineno);
//			switch(node->symbol.value->generic.type) {
//				case NT_VARIABLE:;
//					struct astnode_variable *var = &node->symbol.value->variable;
//					struct astnode_declspec *declspec = &var->declspec->declspec;
//					switch(1)
//					{
//						case DT_FN:
//							fprintf(outfile, "%s function returning:\n", print_sc(declspec->sc));
//						default:
//							fprintf(outfile, "variable with stgclass %s of type:\n", print_sc(declspec->sc));
//					}
//
//
//					// print declarator
//					print_symbol(var->declarator, depth+1);
//
//					// print typequallist
//					print_typequallist(declspec->tq);
//					INDENT(depth+2);
//					print_type(declspec->ts, depth+1);
//					break;
//
//				case NT_TS_STRUCT_UNION:
//					print_type(node->symbol.value, depth+1);
//					break;
//
//				// case NT_LABEL:
//			}
//
//			break;
//
//	}
//

//}

void print_typequallist(union astnode *node)
{
	if (!node) {
		return;
	}
	unsigned char tq = node->tq.qual;
	
	char tmp[30] = {0};
	if (tq & TQ_CONST)
		strcat(tmp, "const ");
	if (tq & TQ_RESTRICT)
		strcat(tmp, "restrict ");
	if (tq & TQ_VOLATILE)
		strcat(tmp, "volatile ");
	fprintf(stdout, "%s", tmp);
	// return tmp;
}


//Function to help print storage class
char *print_sc(union astnode *scspec_node)
{
	enum sc_spec scspec;

	if (!scspec_node) {
		return "";
	}

	scspec = scspec_node->sc.scspec;

	switch(scspec)
	{
		case SC_EXTERN:
			return "extern";
			break; 
		case SC_STATIC:
			return "static";
			break; 
		case SC_AUTO:
			return "auto";
			break;
		case SC_REGISTER:
			return "register";
			break;
		// default:;
		// 	// if in global scope
		// 	struct scope *curscope = get_current_scope();
		// 	if(curscope->type == ST_FILE)
		// 		return "extern";
			// TODO: automatic scope
	}
}

//function to help print scope
char *print_scope(enum scope_type st)
{
	switch(st)
	{
		case ST_FILE:
			return "global";
			break;
		
		// "fake" scope for printing purposes
		case ST_STRUCTUNION:
			return "struct/union";
			break;

		case ST_BLOCK:
			return "block";
			break;
		
		//ST_FUNC, ST_BLOCK, ST_PROTO
	}

}

// insert variable into symbol table
void insert_into_symtab(union astnode *decl_node, union astnode *declspec_node,
	enum name_space ns)
{
	struct astnode_decl *decl;
	char *ident;

	// reverse declarator since components were inserted backwards
	decl_finalize(decl_node, declspec_node);
	decl = &decl_node->decl;
	ident = decl->ident;

	fprintf(stdout, "Declaring new variable with ident %s\n", ident);

	decl_print(decl->components, 0);

//	char *ident;
//	union astnode *symbol, *iter, *var;
//
//	// get declarator name/identifier
//	iter = declarator->declarator.dirdeclarator;
//	while (iter->generic.type != NT_IDENT) {
//		iter = iter->dirdeclarator.ident;
//	}
//	ident = iter->ident.ident;
//
//	// if declarator has a pointer, then make it point to the correct type
//	if (declarator->declarator.pointer) {
//		iter = declarator->declarator.pointer;
//		while (iter->ptr.to) {
//			iter = iter->ptr.to;
//		}
//		iter->ptr.to = declspec;
//	}
//
//	// fill in declspec missing values with their defaults
//	fill_defaults(declspec);
//
//	ALLOC(var);
//	var->variable.type = NT_VARIABLE;
//	var->variable.declspec = declspec;
//	var->variable.declarator = declarator;
//
//	// create symbol and insert
//	ALLOC(symbol);
//	symbol->symbol.type = NT_SYMBOL;
//	symbol->symbol.ident = ident;
//	symbol->symbol.ns = ns;
//	symbol->symbol.value = var;
//
//	// TODO: if declspec has unspecified parts, fill them in with their
//	// context-specific defaults
//	// e.g., if storage class is unspecified, it should be set to extern if
//	// global scope, otherwise auto
//
//	print_symbol(symbol, 0);
//
//	#if DEBUG
//	printf("Declaring symbol %s with type %d\n", ident,
//		declspec->declspec.ts->ts_scalar.basetype);
//	#endif
//
//	scope_insert(ident, ns, symbol);
}

void fill_defaults(union astnode *declspec)
{
	// Check if null
	if(!declspec->declspec.sc)
	{
	//Check if global scope, set default to extern
		struct scope *curscope = get_current_scope();
		if(curscope->type == ST_FILE)
			ALLOC_SET_SCSPEC(declspec->declspec.sc, SC_EXTERN);

	}
	//set type qual
	if(!declspec->declspec.tq)
		ALLOC_SET_TQSPEC(declspec->declspec.tq, 0);
	
	
}

