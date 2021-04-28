/*
 * ast.h
 *
 * Core definitions for the abstract syntax tree.
 */

#ifndef AST_H
#define AST_H

#include "location.h"
#include "semval.h"
#include "types_common.h"

// If you want to add a new astn type, add it to the enum, add the definition,
// add it to the union in struct astn, and add it to print_ast in ast.c
enum astn_types {
    ASTN_ASSIGN,
    ASTN_NUM,
    ASTN_IDENT,
    ASTN_STRLIT,
    ASTN_BINOP,
    ASTN_FNCALL,
    ASTN_SELECT,
    ASTN_UNOP,
    ASTN_SIZEOF,
    ASTN_TERN,
    ASTN_LIST,
    ASTN_TYPESPEC,
    ASTN_TYPEQUAL,
    ASTN_STORSPEC,
    ASTN_TYPE,
    ASTN_DECL,
    ASTN_FNDEF
};

struct astn_assign { // could have been binop but separated for clarity
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
    int argcount;
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

struct astn_list {
    struct astn *me, *next;
};

struct astn_typespec {
    bool is_tagtype;
    union {
        enum typespec spec;
        struct st_entry* symbol;
    };
    struct astn *next;
};

struct astn_typequal {
    enum typequal qual;
    struct astn *next;
};

struct astn_storspec {
    enum storspec spec;
    struct astn *next;
};

struct st_entry;
struct astn_type {
    bool is_derived; // refactor these two into an enum
    bool is_tagtype;
    union {
        struct {
            enum der_types type;
            struct astn *target;
            struct astn *size; // rename?
        } derived;
        struct {
            enum scalar_types type;
            bool is_unsigned;
        } scalar;
        struct {
            enum tagtypes type;
            struct st_entry *symbol;
        } tagtype;
    };
    bool is_volatile;
    bool is_const;
    bool is_restrict;
    bool is_atomic;
};

// as described in parser.y @ decl
struct astn_decl {
    YYLTYPE context;
    struct astn *specs, *type;
};

struct astn_fndef {
    struct astn* decl;
    struct astn* param_list;
};

typedef struct astn {
    enum astn_types type;
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
        struct astn_list astn_list;
        struct astn_typespec astn_typespec;
        struct astn_typequal astn_typequal;
        struct astn_storspec astn_storspec;
        struct astn_type astn_type;
        struct astn_decl astn_decl;
        struct astn_fndef astn_fndef;
    };
} astn;

astn* astn_alloc(enum astn_types type);
void print_ast(const astn *n);

astn *cassign_alloc(int op, astn *left, astn *right);
astn *binop_alloc(int op, astn *left, astn *right);
astn *unop_alloc(int op, astn *target);

astn *list_alloc(astn* me);
astn *list_append(astn *new, astn *head);

astn *list_alloc(astn* me);
astn *list_next(astn* cur);
astn *list_data(astn* n);
unsigned list_measure(const astn *head);

astn *typespec_alloc(enum typespec spec);
astn *typequal_alloc(enum typequal spec);
astn *storspec_alloc(enum storspec spec);
astn *dtype_alloc(astn* target, enum der_types type);

astn *decl_alloc(astn *specs, astn *type, YYLTYPE context);
astn *strunion_alloc(struct st_entry* symbol);

astn *fndef_alloc(astn* decl, astn* param_list);

void set_dtypechain_target(astn* top, astn* target);
void reset_dtypechain_target(astn* top, astn* target);
void merge_dtypechains(astn *parent, astn *child);
astn* get_dtypechain_target(astn* top);


#endif
