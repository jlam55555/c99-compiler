/**
 * sizeof operator to determine size of an AST node
 *
 * Assumes the architecture is similar to common x86_64 implementations.
 */

#ifndef SIZEOFH
#define SIZEOFH

#include <parser/astnode.h>

/**
 * Returns the size of a symbol. Calls astnode_sizeof_type() on the symbol's
 * type. Supposed to be equivalent to `sizeof node`.
 *
 * @param node		symbol (decl) astnode to take the size of
 * @return		size of the symbol, in bytes
 */
unsigned astnode_sizeof_symbol(union astnode *node);

/**
 * Returns the size of a type. Type can be a scalar, struct/union, or array.
 * Supposed to be equivalent to `sizeof(typename)`.
 *
 * @param type		type to take the size of; should be the decl.components
 * 			linked list (which is a superset of typespecs)
 * @return		size of the type, in bytes
 */
unsigned astnode_sizeof_type(union astnode *type);

#endif
