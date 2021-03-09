#ifndef AST_H
#define AST_H

#include "semval.h"

enum astn_type {
    ASTN_ASSIGN,
    ASTN_NUM,
    ASTN_IDENT,
    ASTN_STRLIT,
    ASTN_BINOP,
    ASTN_DEREF,
    ASTN_FNCALL,
    ASTN_SELECT,
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
    char op;
    struct astn *left, *right;
};

struct astn_deref {
    struct astn *target;
};

struct astn_fncall {
    struct astn *fn;
    struct astn *args; // should be an arg_expr_list
};

struct astn_select {
    struct astn *parent, *member;
};

typedef struct astn {
    enum astn_type type;
    union {
        struct astn_assign astn_assign;
        struct astn_num astn_num;
        struct astn_ident astn_ident;
        struct astn_strlit astn_strlit;
        struct astn_binop astn_binop;
        struct astn_deref astn_deref;
        struct astn_fncall astn_fncall;
        struct astn_select astn_select;
    };
} astn;

astn* astn_alloc();
void print_ast(astn *n);

#endif