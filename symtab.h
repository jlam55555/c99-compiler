#ifndef SYMTABH
#define SYMTABH

enum name_space { NS_TAG, NS_LABEL, NS_BLOCK, NS_IDENT }; 
enum scope_type { ST_FILE, ST_FUNC, ST_BLOCK, ST_PROTO };

struct symtab {
	union astnode **bs;
	int size, capacity;
};

struct scope {
	enum scope_type type;
	struct symtab ns[4];
};

void scope_push(enum scope_type type);
void scope_pop(void);
void scope_insert(char *ident, enum name_space ns, union astnode *node);
union astnode *scope_lookup(char *ident, enum name_space ns);
struct scope *get_scope(char *ident, enum name_space ns);

#endif