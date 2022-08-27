/*
 * ast.h
 *
 * Core definitions for the abstract syntax tree.
 */

#ifndef AST_H
#define AST_H

#include "ir_types.h"
#include "location.h"
#include "macro_util.h"
#include "semval.h"
#include "types_common.h"
#include "util.h"

// Note that struct st_entry* is used in this file as a forward reference.

// If you want to add a new astn type, add it here, add the definition,
// add it to the union in struct astn, and add it to print_ast in ast_print.c

// KIND_UNDEF, KIND_MAX, and NOOP have no associated struct
#define FOREACH_ASTN_KIND(MAKER)    \
    MAKER(ASTN_KIND_UNDEF),     \
    MAKER(ASTN_ASSIGN),         \
    MAKER(ASTN_NUM),            \
    MAKER(ASTN_IDENT),          \
    MAKER(ASTN_STRLIT),         \
    MAKER(ASTN_BINOP),          \
    MAKER(ASTN_FNCALL),         \
    MAKER(ASTN_SELECT),         \
    MAKER(ASTN_UNOP),           \
    MAKER(ASTN_SIZEOF),         \
    MAKER(ASTN_TERN),           \
    MAKER(ASTN_LIST),           \
    MAKER(ASTN_TYPESPEC),       \
    MAKER(ASTN_TYPEQUAL),       \
    MAKER(ASTN_STORSPEC),       \
    MAKER(ASTN_TYPE),           \
    MAKER(ASTN_DECL),           \
    MAKER(ASTN_FNDEF),          \
    MAKER(ASTN_ELLIPSIS),       \
    MAKER(ASTN_COMPOUNDSTMT),   \
    MAKER(ASTN_DECLREC),        \
    MAKER(ASTN_SYMPTR),         \
    MAKER(ASTN_IFELSE),         \
    MAKER(ASTN_SWITCH),         \
    MAKER(ASTN_WHILELOOP),      \
    MAKER(ASTN_FORLOOP),        \
    MAKER(ASTN_GOTO),           \
    MAKER(ASTN_BREAK),          \
    MAKER(ASTN_CONTINUE),       \
    MAKER(ASTN_RETURN),         \
    MAKER(ASTN_LABEL),          \
    MAKER(ASTN_CASE),           \
    MAKER(ASTN_QTEMP),          \
    MAKER(ASTN_QBBNO),          \
    MAKER(ASTN_QTYPE),          \
    MAKER(ASTN_NOOP),           \
    MAKER(ASTN_KIND_MAX)

enum astn_types {
    FOREACH_ASTN_KIND(GENERATE_ENUM)
};



struct astn_assign { // could have been binop but separated for clarity
    // struct astn left, *right;
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
        struct st_entry *symbol;
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
            struct astn *param_list;
            struct symtab *scope;
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
    struct astn *decl;
    struct astn *param_list;
    struct symtab* scope;
};

// no astn_ellipsis struct

struct astn_compoundstmt {
    YYLTYPE begin, end;
    struct astn *list;
};

// record of a declaration having occurred
struct astn_declrec {
    struct st_entry *e;
    struct astn *init;
};

// st_entry pointer from resolved idents
// similar above but separated for clarity
struct astn_symptr {
    struct st_entry *e;
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
    struct astn *ident;
};

struct astn_break {
    struct astn *dummy;
};

struct astn_continue {
    struct astn *dummy;
};

struct astn_return {
    struct astn *ret;
};

struct astn_label {
    struct astn *ident, *statement;
};

struct astn_case {
    struct astn *case_expr, *statement;
};

struct astn_qtemp {
    unsigned tempno;
    struct astn *qtype;
};

struct BB;
struct astn_qbbno {
    struct BB* bb;
};

struct astn_qtype {
    ir_type_E qtype;
};

// uppercase member names are a style decision; they're clear and they also
// allow us to have members like .Sizeof, .Return, etc without clobbering
// the names to avoid conflicting with keywords.

struct astn {
    enum astn_types type;
    union {
        struct astn_assign Assign;
        struct astn_num Num;
        struct astn_ident Ident;
        struct astn_strlit Strlit;
        struct astn_binop Binop;
        struct astn_fncall Fncall;
        struct astn_select Select;
        struct astn_unop Unop;
        struct astn_sizeof Sizeof;
        struct astn_tern Tern;
        struct astn_list List;
        struct astn_typespec Typespec;
        struct astn_typequal Typequal;
        struct astn_storspec Storspec;
        struct astn_type Type;
        struct astn_decl Decl;
        struct astn_fndef Fndef;
        struct astn_compoundstmt Compoundstmt;
        struct astn_declrec Declrec;
        struct astn_symptr Symptr;
        struct astn_ifelse Ifelse;
        struct astn_switch Switch;
        struct astn_whileloop Whileloop;
        struct astn_forloop Forloop;
        struct astn_goto Goto;
        struct astn_return Return;
        struct astn_label Label;
        struct astn_case Case;
        struct astn_qtemp Qtemp;
        struct astn_qbbno Qbbno;
        struct astn_qtype Qtype;
    };
};

typedef struct astn* astn;
typedef const struct astn* const_astn;

astn astn_alloc(enum astn_types type);
const char *astn_kind_str(const_astn a);

astn simple_constant_alloc(int num);

astn cassign_alloc(int op, astn left, astn right);
astn binop_alloc(int op, astn left, astn right);
astn unop_alloc(int op, astn target);

astn list_alloc(astn me);
astn list_append(astn new, astn head);

astn list_alloc(astn me);
astn list_next(const_astn cur);
astn list_data(const_astn n);
void list_reverse(astn *l);
unsigned list_measure(const_astn head);

astn typespec_alloc(enum typespec spec);
astn typequal_alloc(enum typequal spec);
astn storspec_alloc(enum storspec spec);
astn dtype_alloc(astn target, enum der_types type);

astn decl_alloc(astn specs, astn type, astn init, YYLTYPE context);
astn strunion_alloc(struct st_entry *symbol);

astn fndef_alloc(astn decl, astn param_list, struct symtab* scope);
astn declrec_alloc(struct st_entry *e, astn init);
astn symptr_alloc(struct st_entry *e);

astn ifelse_alloc(astn cond_s, astn then_s, astn else_s);
astn whileloop_alloc(astn cond_s, astn body_s, bool is_dowhile);
astn forloop_alloc(astn init, astn condition, astn oneach, astn body);

astn do_decl(astn decl);

void set_dtypechain_target(astn top, astn target);
void reset_dtypechain_target(astn top, astn target);
void merge_dtypechains(astn parent, astn child);

astn get_dtypechain_last_link(astn top);
astn get_dtypechain_target(astn top);
const char *get_dtypechain_ident(astn d);

astn qtemp_alloc(int tempno, astn qtype);
astn qtype_alloc(ir_type_E t);

void print_ast(); // fwd-decl
#define ast_check(node, asttype, msg)                                       \
    {                                                                       \
        if (node->type != asttype) {                                        \
            eprintf("FAILED ASTN CHECK AT %s:%d: ", __FILE__, __LINE__);    \
            eprintf("Expected type %s and got type %s:\n",                  \
                    #asttype, astn_kind_str(node));                         \
            print_ast(node);                                                \
            die(msg);                                                       \
        }                                                                   \
    }

#endif
