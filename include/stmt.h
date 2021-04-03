#ifndef STATEMENTH
#define STATEMENTH

#include "common.h"
#include "astnode.h"
//label statements
struct astnode_stmt_label {
    _ASTNODE;
    union astnode *label;
    union astnode *body;
};

struct astnode_stmt_case {
    _ASTNODE;
};

struct astnode_stmt_compound {
    _ASTNODE;
    union astnode *body;
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

#define ALLOC_STMT_IFELSE(var, condition, if_stmt, else_stmt)\
    ALLOC_TYPE(var, NT_STMT_IFELSE);\
    var->stmt_if_else.cond = condition;\
    var->stmt_if_else.ifstmt = if_stmt;\
    var->stmt_if_else.elsestmt = else_stmt;

#define ALLOC_STMT_SWITCH(var, condition, statement)\
    ALLOC_TYPE(var, NT_STMT_SWITCH);\
    var->stmt_switch.cond = condition;\
    var->stmt_switch.body = statement;

#define ALLOC_STMT_WHILE(var, condition, statement)\
    ALLOC_TYPE(var, NT_STMT_WHILE);\
    var->stmt_while.cond = condition;\
    var->stmt_while.body = statement;

#define ALLOC_STMT_DO_WHILE(var, condition, statement)\
    ALLOC_TYPE(var, NT_STMT_DO_WHILE);\
    var->stmt_do_while.cond = condition;\
    var->stmt_do_while.body = statement; 

#define ALLOC_STMT_FOR(var, initial, condition, upd, statement)\
    ALLOC_TYPE(var, NT_STMT_FOR);\
    var->stmt_for.init = initial;\
    var->stmt_for.cond = condition;\
    var->stmt_for.update = upd;\
    var->stmt_for.body = statement;

#define ALLOC_STMT_GOTO(var, lbl)\
    ALLOC_TYPE(var, NT_STMT_GOTO);\
    /*TODO: have to look up label in symbol table*/\
    /*var->stmt_goto.label = lbl;*/

#define ALLOC_STMT_BREAK_CONT(var, BREAK_CONT)\
    ALLOC_TYPE(var, BREAK_CONT);

#define ALLOC_STMT_RETURN(var, ret)\
    ALLOC_TYPE(var, NT_STMT_RETURN);\
    var->stmt_return.rt = ret;


#endif