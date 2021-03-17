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

#endif	// PRINTUTILSH