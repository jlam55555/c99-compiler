/**
 * 	Collection of functions to print out various internal representations.
 */

#ifndef PRINTUTILSH
#define PRINTUTILSH

/**
 * print out struct/union definition recursively
 *
 * @param node 	struct/union typespec
 */
void print_structunion_def(union astnode *node);

/**
 * print out typespec (scalar or struct/union)
 *
 * @param node		astnode_ts_* object
 * @param depth		indenting depth
 */
void print_typespec(union astnode *node, int depth);

/**
 * recursively print declarator components; initiate by calling on the
 * component list of a astnode_decl object
 *
 * @param component	ll of astnode_decl_*
 * @param depth		indenting depth
 */
void print_declarator(union astnode *component, int depth);

/**
 * print type qualifier list
 *
 * @param node 		astnode_typequal object
 */
void print_typequal(union astnode *node);

/**
 * print storage class
 *
 * @param node		astnode_storageclass object
 */
void print_storageclass(union astnode *node);

/**
 * print scope
 *
 * @param scope 	scope object
 */
void print_scope(struct scope *scope);

/**
 * print variable: declaration specifiers, scope, and declarator
 *
 * @param node 		astnode_decl (declaration) object
 */
void print_symbol(union astnode *node);

#endif	// PRINTUTILSH