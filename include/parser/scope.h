/**
 * 	Scope management functions. Allows creating/popping slopes. Note that
 * 	the global (file) scope should be created by calling
 * 	`scope_push(ST_FILE)` before calling yyparse(). The scopes are stored
 * 	recursively on a stack, and each scope contains the three namespace
 * 	types. Does not include "mini symbol tables" used for structs/unions --
 * 	these are handled separately in structunion.c. Uses symtab.c hashtable
 * 	implementation.
 */

#ifndef SCOPEH
#define SCOPEH
#include <parser/symtab.h>

enum name_space { NS_TAG, NS_LABEL, NS_IDENT };
enum scope_type { ST_FILE, ST_FUNC, ST_BLOCK, ST_PROTO, ST_STRUCTUNION };

struct scope {
	enum scope_type type;
	struct symtab ns[3];

	// holds a linked list of all of the symbols in the scope, for
	// prototype/function scopes; need an ordered list during quad
	// generation to build offsets
	union astnode *symbols_ll;

	// for debugging/printing purposes
	char *filename;
	int lineno;
};

/**
 * Push a new scope onto the stack. For ST_FILE, ST_FUNC, and ST_BLOCK, can
 * set type to 0 and it will automatically be determined. For ST_STRUCTUNION
 * and ST_PROTO it needs to be set explicitly.
 *
 * @param type		see notes above
 */
void scope_push(enum scope_type type);

/**
 * Pops the top scope.
 */
void scope_pop(void);

/**
 * Insert a symbol at the current scope (or at the nearest function scope, for
 * labels), and returns the scope
 *
 * @param ident		symbol name
 * @param ns		namespace to insert symbol
 * @param node		astnode representing symbol
 * @return		scope which the symbol is inserted into, or NULL if
 * 			redeclaration of extern
 */
struct scope *scope_insert(char *ident, enum name_space ns,
	union astnode *node);

/**
 * Recursive lookup of ident up the scope stack
 *
 * @param ident 	symbol name
 * @param ns 		namespace to search in
 * @return 		symbol if found, otherwise NULL
 */
union astnode *scope_lookup(char *ident, enum name_space ns);

/**
 * Recursive lookup of ident up the scope stack, returning the scope in which
 * the symbol is found (or NULL if the symbol was not found)
 *
 * @param ident		symbol name
 * @param ns		namespace to search in
 * @return		scope containing symbol if symbol found, otherwise NULL
 */
struct scope *get_scope(char *ident, enum name_space ns);

/**
 * Returns a pointer to the current scope
 *
 * @return		pointer to current scope
 */
struct scope *get_current_scope();

/**
 * sets a flag to indicate that the current function declaration is a function
 * definition so the prototype should be promoted and not discarded after the
 * current function declarator (somewhat kludgey)
 */
void scope_set_fndef();

/**
 * gets the enclosing function scope
 * 
 * @return		the enclosing function scope, or NULL if not in a
 * 			function scope
 */
struct scope *get_fn_scope();

/**
 * associates a function declarator with its associated function scope
 * 
 * @param fn_decl	the function with which to associate the previous
 * 			fn scope 
 */
void associate_fn_with_scope(union astnode *fn_decl);

#endif	// SCOPEH