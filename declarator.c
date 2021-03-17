#include "astnodegeneric.h"
#include "astnode.h"
#include "declarator.h"

void declarator_new(union astnode *node)
{
	ALLOC(node);
}
