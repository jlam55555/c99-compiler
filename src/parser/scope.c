#include <parser/astnode.h>
#include <parser/scope.h>
#include <parser/parser.h>
#include <lexer/errorutils.h>
#include <stdio.h>

static struct scope *scope_stack = NULL;
static int scope_pos = -1, scope_stack_capacity = 0;

// special flag used for transferring prototype scopes to function scopes
// if definition (function body) follows function declaration
static int prototype_hold = 0, is_fndef = 0;
void scope_set_fndef()
{
	is_fndef = 1;
}

// scope begun, create namespaces/symbol tables for it
void scope_push(enum scope_type type)
{
	int i;
	struct symtab *st;

	// if prototype_hold is set and a new non-function scpoe is created,
	// then clear held prototype
	if (prototype_hold && type) {
		// shouldn't happen (prototype_hold should only be set when
		// scope_pos==1), just a check
		if (scope_pos != 1) {
			yyerror_fatal("unexpected scope position");
		}
		scope_stack[1].type = ST_FILE;
		scope_pop();
		prototype_hold = 0;
	}

	// decide what to do if scope type is not specified: used for
	// file/function/block scopes (and not for proto/structunion scopes)
	if (!type) {
		// no scope on stack
		if (scope_pos < 0) {
			type = ST_FILE;
		}
		// top-level function scope, promote existing prototype scope
		else if (scope_pos == 1 && prototype_hold) {
			// indicate all variables in prototype scope are
			// from prototype scope
			for (i = 0, st = &scope_stack[1].ns[NS_IDENT];
				i < st->capacity; ++i) {
				if (st->bs[i]) {
					st->bs[i]->value->decl.is_proto = 1;
				}
			}

			scope_stack[1].type = ST_FUNC;
			prototype_hold = 0;
			// TODO: check that there are no abstract declarators?
			return;
		}
		// everything else: block scope
		else {
			type = ST_BLOCK;
		}
	}

	// grow scope_stack if necessary
	if (scope_stack_capacity <= scope_pos + 1) {
		// stack has never been initialized, give it an initial value
		if (!scope_stack_capacity) {
			scope_stack_capacity = 16;
		} else {
			scope_stack_capacity *= 2;
		}
		scope_stack = realloc(scope_stack,
			scope_stack_capacity * sizeof(struct scope));
	}

	// create scope
	struct scope *scope = &scope_stack[++scope_pos];
	for (i = 0; i < 3; i++) {
		symtab_init(&scope->ns[i]);
	}

	// set scope parameters
	scope->type = type;
	scope->filename = strdup(filename);
	scope->lineno = lineno;
}

// scope ended, destroy it
void scope_pop(void)
{
	int i;

	if (!scope_pos) {
		yyerror_fatal("popping empty scope stack");
	}

	// prevent deleting top-level prototype scope right away, because
	// it may need to be transferred to a function scope
	if (scope_pos == 1 && scope_stack[scope_pos].type == ST_PROTO) {
		prototype_hold = 1;
		return;
	}

	// destroy scope
	for (i = 0; i < 3; i++) {
		symtab_destroy(&scope_stack[scope_pos].ns[i]);
	}
	--scope_pos;
}

// helper function to shorten scope_insert(); allows redeclaration of
// (compatible) variables/functions of extern storage class
//
// this is done before declspec_fill_defaults() (called in scope_insert), 
// so need to explicitly check that the variable is extern here, or infer that
// it is extern if in global scope
//
// TODO: see TODO's
static int extern_redeclaration(char *ident, enum name_space ns,
	union astnode *node)
{
	union astnode *search;

	if (!(search = symtab_lookup(&scope_stack[is_fndef?0:scope_pos].ns[ns],
			ident))) {
		// not already defined, this wouldn't be a redeclaration
		return 0;
	}
		
	// shouldn't be a warning but here for debugging
#ifdef DEBUG
	yyerror("previously-declared extern variable");
#endif
	
	// TODO: check that types are compatible

	// TODO: should also "narrow type" if applicable
	// (i.e., if specifies narrower fn params)

	// replace input decl with old declaration
	// TODO: discard new declaration
	*node = *search;

	// redeclaration
	return 1;
}

// inserts symbol at enclosing scope (or at function scope for labels)
struct scope *scope_insert(char *ident, enum name_space ns, union astnode *node)
{
	struct scope *scope;

	// declaration while holding means break holding
	if (prototype_hold && !is_fndef) {
		// shouldn't happen (prototype_hold should only be set when
		// scope_pos==1), just a check
		if (scope_pos != 1) {
			yyerror_fatal("unexpected scope position");
		}
		scope_stack[1].type = ST_FILE;
		scope_pop();
		prototype_hold = 0;
	}

	// allow for redeclaration of extern function; see extern_redeclaration
	// check if ident type (in "everything else" namespace)
	if (ns == NS_IDENT && NT(node) == NT_DECL) {
		union astnode *sc = node->decl.declspec->declspec.sc;

		// check if extern
		if ((sc && sc->sc.scspec == SC_EXTERN)
			|| (!sc && (!scope_pos || is_fndef))) {

			// allow for redeclaration if already defined
			if (extern_redeclaration(ident, ns, node)) {
				is_fndef = 0;
				return NULL;
			}
		}
	}

	scope = &scope_stack[is_fndef?0:scope_pos];
	symtab_insert(&scope->ns[ns], ident, node);
	is_fndef = 0;
	return scope;
}

// traverses up the stack to lookup a symbol
union astnode *scope_lookup(char *ident, enum name_space ns)
{
	int current_scope;
	union astnode *search = NULL;

	for (current_scope = scope_pos;
		current_scope >= 0 && !(search =
		symtab_lookup(&scope_stack[current_scope].ns[ns], ident));
		--current_scope);

	return search;
}

// do same as above but return scope object
struct scope *get_scope(char *ident, enum name_space ns)
{
	int current_scope;
	union astnode *search = NULL;

	for (current_scope = scope_pos;
		current_scope >= 0 && !(search =
		symtab_lookup(&scope_stack[current_scope].ns[ns], ident));
		--current_scope);

	return current_scope >= 0 ? &scope_stack[current_scope] : NULL;
}

struct scope *get_current_scope()
{
	return &scope_stack[scope_pos];
}