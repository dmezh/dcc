#ifndef AST_H
#define AST_H

#include "semval.h"

enum astn_type {
    ASTN_ASSIGN,
    ASTN_NUM,
    ASTN_IDENT,
    ASTN_STRLIT,
    ASTN_BINOP,
    ASTN_FNCALL,
    ASTN_SELECT,
    ASTN_UNOP,
    ASTN_SIZEOF,
    ASTN_TERN
};

struct astn_assign {
    // struct astn* left, *right;
    struct astn *left, *right;
};

// I'm questioning whether yystype could just be one of the union vals in struct astn instead
struct astn_num {
    struct number number;
};

struct astn_ident {
    char* ident;
};

struct astn_strlit {
    struct strlit strlit;
};

struct astn_binop {
    int op;
    struct astn *left, *right;
};

struct astn_fncall {
    struct astn *fn;
    struct astn *args; // should be an arg_expr_list
};

struct astn_select {
    struct astn *parent, *member;
};

struct astn_unop {
    int op;
    struct astn *target;
};

struct astn_sizeof {
    struct astn *target;
};

struct astn_tern {
    struct astn *cond, *t_then, *t_else;
};

typedef struct astn {
    enum astn_type type;
    union {
        struct astn_assign astn_assign;
        struct astn_num astn_num;
        struct astn_ident astn_ident;
        struct astn_strlit astn_strlit;
        struct astn_binop astn_binop;
        struct astn_fncall astn_fncall;
        struct astn_select astn_select;
        struct astn_unop astn_unop;
        struct astn_sizeof astn_sizeof;
        struct astn_tern astn_tern;
    };
} astn;

astn* astn_alloc();
void print_ast(astn *n);

#endif