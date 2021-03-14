#include "structunion.h"
#include "symtab.h"
#include "parser.h"
#include "scope.h"

// stack of union astnodes currently being declared
static union astnode **su_decl_stack;
int su_decl_stack_pos = -1;

union astnode *structunion_new(enum structunion_type type)
{
	// TODO
}

void structunion_set_name(char *ident)
{
	// TODO
}

void structunion_install_member(union astnode *declarator,
	union astnode *specquallist)
{
	// TODO
}

union astnode *structunion_done(enum structunion_complete is_complete)
{
	return NULL;
}