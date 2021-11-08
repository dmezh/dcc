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
// add it to the union in struct astn, and add it to print_ast in ast_print.c
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
    ASTN_FNDEF,
    ASTN_COMPOUNDSTMT,
    ASTN_DECLREC,
    ASTN_SYMPTR,
    ASTN_IFELSE,
    ASTN_SWITCH,
    ASTN_WHILELOOP,
    ASTN_FORLOOP,
    ASTN_GOTO,
    ASTN_BREAK,
    ASTN_CONTINUE,
    ASTN_RETURN,
    ASTN_LABEL,
    ASTN_CASE,
    ASTN_QTEMP,
    ASTN_QBBNO,
    ASTN_NOOP // has no associated struct
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
    const char* ident;
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
    struct astn *init;
};

struct astn_fndef {
    struct astn* decl;
    struct astn* param_list;
    struct symtab* scope;
};

struct astn_compoundstmt {
    YYLTYPE begin, end;
    struct astn *list;
};

// record of a declaration having occurred
struct astn_declrec {
    struct st_entry* e;
    struct astn* init;
};

// st_entry pointer from resolved idents
// similar above but separated for clarity
struct astn_symptr {
    struct st_entry* e;
};

struct astn_ifelse {
    struct astn *condition_s, *then_s, *else_s;
};

struct astn_switch {
    struct astn *condition, *body;
};

struct astn_whileloop {
    bool is_dowhile;
    struct astn *condition, *body;
};

struct astn_forloop {
    struct astn *init, *condition, *oneach, *body;
};

struct astn_goto {
    struct astn* ident;
};

struct astn_break {
    struct astn* dummy;
};

struct astn_continue {
    struct astn* dummy;
};

struct astn_return {
    struct astn* ret;
};

struct astn_label {
    struct astn* ident, *statement;
};

struct astn_case {
    struct astn* case_expr, *statement;
};

struct astn_qtemp {
    unsigned tempno, size;
    int stack_offset;
};

struct BB;
struct astn_qbbno {
    struct BB* bb;
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
        struct astn_compoundstmt astn_compoundstmt;
        struct astn_declrec astn_declrec;
        struct astn_symptr astn_symptr;
        struct astn_ifelse astn_ifelse;
        struct astn_switch astn_switch;
        struct astn_whileloop astn_whileloop;
        struct astn_forloop astn_forloop;
        struct astn_goto astn_goto;
        struct astn_return astn_return;
        struct astn_label astn_label;
        struct astn_case astn_case;
        struct astn_qtemp astn_qtemp;
        struct astn_qbbno astn_qbbno;
    };
} astn;

astn* astn_alloc(enum astn_types type);

astn *cassign_alloc(int op, astn *left, astn *right);
astn *binop_alloc(int op, astn *left, astn *right);
astn *unop_alloc(int op, astn *target);

astn *list_alloc(astn* me);
astn *list_append(astn *new, astn *head);

astn *list_alloc(astn* me);
astn *list_next(const astn* cur);
astn *list_data(const astn* n);
void list_reverse(astn **l);
unsigned list_measure(const astn *head);

astn *typespec_alloc(enum typespec spec);
astn *typequal_alloc(enum typequal spec);
astn *storspec_alloc(enum storspec spec);
astn *dtype_alloc(astn* target, enum der_types type);

astn *decl_alloc(astn *specs, astn *type, astn* init, YYLTYPE context);
astn *strunion_alloc(struct st_entry* symbol);

astn *fndef_alloc(astn* decl, astn* param_list, struct symtab* scope);
astn *declrec_alloc(struct st_entry *e, astn* init);
astn *symptr_alloc(struct st_entry* e);

astn *ifelse_alloc(astn *cond_s, astn *then_s, astn *else_s);
astn *whileloop_alloc(astn* cond_s, astn* body_s, bool is_dowhile);
astn *forloop_alloc(astn *init, astn* condition, astn* oneach, astn* body);

astn *do_decl(astn *decl);

void set_dtypechain_target(astn* top, astn* target);
void reset_dtypechain_target(astn* top, astn* target);
void merge_dtypechains(astn *parent, astn *child);
astn* get_dtypechain_target(astn* top);


#endif
