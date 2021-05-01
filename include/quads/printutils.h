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
 * Prints a single basic block and its successors, recursively
 *
 * notes:
 * - if called on the root note, will print the entire CFG
 * - prevents duplicates by storing a ll of visited bbs
 *
 * @param bb			basic block object to print
 */
void print_basic_block(struct basic_block *bb);

/**
 * print the list of basic blocks stored in bb_ll
 *
 * make sure to call this after calling finalize_bb_ll (quads.c) so that
 * the basic blocks are in the correct order
 */
void print_basic_blocks();

#endif	// QUADPRINTUTILSH
