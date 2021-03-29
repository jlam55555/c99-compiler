#ifndef STATEMENTH
#define STATEMENTH

#include "common.h"

//label statements
struct astnode_stmt_label {
    _ASTNODE;
    union astnode *label;
    union astnode *body;
};

struct astnode_stmt_case {
    _ASTNODE;
};

//selection statements
struct astnode_stmt_if_else {
    _ASTNODE;
    union astnode *cond;
    union astnode *ifstmt, *elsestmt;
};

struct astnode_stmt_switch {
    _ASTNODE;
    union astnode *cond;
    union astnode *body;
};

//Loops
struct astnode_stmt_do_while {
    _ASTNODE;
    union astnode *cond;
    union astnode *body;
};

struct astnode_stmt_while {
    _ASTNODE;
    union astnode *cond;
    union astnode *body;
};

struct astnode_stmt_for {
    _ASTNODE;
    union astnode *init;
    union astnode *cond;
    union astnode *update;
    union astnode *body;
};

//jump statement
struct astnode_stmt_goto {
    _ASTNODE;
    union astnode *label;
};

struct astnode_stmt_break_cont {
    _ASTNODE;
};

struct astnode_stmt_return {
    _ASTNODE;
    union astnode *rt;
};




#endif