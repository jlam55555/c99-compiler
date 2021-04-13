#include <quads/sizeof.h>

unsigned astnode_sizeof_symbol(union astnode *node)
{
	if (NT(node) != NT_DECL) {
		yyerror_fatal("quadgen: sizeof(symbol): not a symbol");
	}

	return astnode_sizeof_type(node->decl.components);
}

unsigned astnode_sizeof_type(union astnode *type)
{
	union astnode *iter;
	unsigned size;

	if (!type) {
		yyerror_fatal("quadgen: sizeof(type): NULL type component");
	}

	switch (NT(type)) {

	// sizeof array is count*sizeof(contained type)
	case NT_DECLARATOR_ARRAY:
		// TODO: for now, assume array expression is an int
		return ((unsigned)type->decl_array.length->num.num.int_val)
			* astnode_sizeof_type(LL_NEXT_OF(type));

	// sizeof (pointer) = width of memory bus (assumption: 8 bytes)
	// function also gets treated like a pointer
	case NT_DECLARATOR_POINTER:
	case NT_DECLARATOR_FUNCTION:
		return 8;

	// sizeof(declspec): get size of typespec
	case NT_DECLSPEC:
		return astnode_sizeof_type(type->declspec.ts);

	// return scalar size
	case NT_TS_SCALAR:
		// this makes the assumption that the types were appropriate
		// vetted in merge_declspec(); these values come from gcc10.2.0:
		// short int: 2; long int: 8; long long int: 8; long double: 16
		switch (type->ts_scalar.modifiers.lls) {
		case LLS_LONG:		return type->ts_scalar.basetype==BT_INT
						     ? 8 : 16;
		case LLS_LONG_LONG:	return 8;
		case LLS_SHORT:		return 2;
		}

		// non-modified scalar types; these values come from gcc10.2.0
		switch (type->ts_scalar.basetype) {
		case BT_UNSPEC:
			yyerror_fatal("quadgen: sizeof(type): unspecified"
				" basetype");
		case BT_VOID:
			yyerror_fatal("quadgen: sizeof(void)");
		case BT_BOOL:
		case BT_CHAR:	return 1;
		case BT_INT:
		case BT_FLOAT: 	return 4;
		case BT_DOUBLE:	return 8;
		}

	// return struct/union size
	case NT_TS_STRUCT_UNION:
		// TODO: would be better to compute offsets when creating
		// 	the struct/union, and also compute padding there;
		// 	this bad implementation doesn't compute padding (but
		// 	really should)
		size = 0;
		LL_FOR(type->ts_structunion.members, iter) {
			size += astnode_sizeof_type(iter->decl.components);
		}
		return size;

	default:
		yyerror_fatal("quadgen: sizeof(type): invalid type component");
		return -1;
	}
}
