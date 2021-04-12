/**
 * Printing utilities for the quad IR
 */

#ifndef QUADPRINTUTILSH
#define QUADPRINTUTILSH

#include <quads/quads.h>

/**
 * Prints a single quad
 *
 * @param quad			quad object to print
 */
void print_quad(struct quad *quad);

/**
 * Prints a single basic block
 *
 * @param bb			basic block object to print
 */
void print_basic_block(struct basic_block *bb);

/**
 * Prints a linked-list of basic blocks, without repetition (to prevent
 * infinite loops)
 *
 * @param bb 			head of basic block linked-list
 */
void print_basic_blocks(struct basic_block *bb);

#endif	// QUADPRINTUTILSH
