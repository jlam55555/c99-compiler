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
 * print out typespec
 *
 * @param node
 * @param depth
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

#endif	// PRINTUTILSH