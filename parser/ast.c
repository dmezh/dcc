/*
 * ast.c
 * 
 * This file contains functions for the construction and manipulation of the AST.
 * x_alloc functions all allocate, potentially populate, and return new AST nodes.
 * All AST node allocation should be done internally with astn_alloc. 
 */

#include "ast.h"

#include <stdio.h>
#include <stdlib.h>

#include "symtab.h"
#include "util.h"

/*
 * Allocate single astn safely.
 * The entire compiler relies on the astn type being set, so we must do that.
 */
astn* astn_alloc(enum astn_types type) {
    astn *n = safe_calloc(1, sizeof(astn));
    n->type = type;
    return n;
}

/*
 * allocate complex assignment (*=, /=, etc) - 6.5.16
 */
astn *cassign_alloc(int op, astn *left, astn *right) {
    astn *n=astn_alloc(ASTN_ASSIGN);
    n->astn_assign.left=left;
    n->astn_assign.right=binop_alloc(op, left, right);
    return n;
}

/*
 * allocate binary operation
 */
astn *binop_alloc(int op, astn *left, astn *right) {
    astn *n=astn_alloc(ASTN_BINOP);
    n->astn_binop.op=op;
    n->astn_binop.left=left;
    n->astn_binop.right=right;
    return n;
}

/*
 * allocate unary operation
 */
astn *unop_alloc(int op, astn *target) {
    astn *n=astn_alloc(ASTN_UNOP);
    n->astn_unop.op=op;
    n->astn_unop.target=target;
    return n;
}

/*
 * allocate a list node
 */
astn *list_alloc(astn *me) {
    astn *l=astn_alloc(ASTN_LIST);
    l->astn_list.me=me;
    l->astn_list.next=NULL;
    return l;
}

/*
 * allocate a list node and append it to end of existing list (from head)
 * return: ptr to new node
 */
//              (arg to add)(head of ll)
astn *list_append(astn* new, astn *head) {
    astn *n=list_alloc(new);
    while (head->astn_list.next) head=head->astn_list.next;
    head->astn_list.next = n;
    return n;
}

/*
 *  get next node of list
 */
astn *list_next(astn* cur) {
    return cur->astn_list.next;
}

/*
 *  get current data element
 */
astn *list_data(astn* n) {
    return n->astn_list.me;
}

// DSA shit
// https://www.geeksforgeeks.org/reverse-a-linked-list/
void list_reverse(astn **l) {
    struct astn *prev=NULL, *current=*l, *next=NULL;
    while (current) {
        next = list_next(current);
        current->astn_list.next = prev;
        prev = current;
        current = next;
    }
    *l = prev;
}

/*
 * return length of AST list starting at head
 */
unsigned list_measure(const astn *head) {
    int c = 0;
    while ((head=head->astn_list.next)) {
        c++;
    }
    return c + 1;
}

/*
 * allocate type specifier node
 */
astn *typespec_alloc(enum typespec spec) {
    astn *n=astn_alloc(ASTN_TYPESPEC);
    n->astn_typespec.spec = spec;
    n->astn_typespec.next = NULL;
    //fprintf(stderr, "ALLOCATED TSPEC\n");
    return n;
}

/*
 * allocate type qualifier node
 */
astn *typequal_alloc(enum typequal qual) {
    astn *n=astn_alloc(ASTN_TYPEQUAL);
    n->astn_typequal.qual = qual;
    n->astn_typespec.next = NULL;
    return n;
}

/*
 * allocate storage specifier node
 */
astn *storspec_alloc(enum storspec spec) {
    astn *n=astn_alloc(ASTN_STORSPEC);
    n->astn_storspec.spec = spec;
    n->astn_storspec.next = NULL;
    return n;
}

/*
 * allocate derived type node
 */
astn *dtype_alloc(astn *target, enum der_types type) {
    astn *n=astn_alloc(ASTN_TYPE);
    n->astn_type.is_derived = true;
    n->astn_type.derived.type = type;
    n->astn_type.derived.target = target;
    //fprintf(stderr, "ALLOCATED DTYPE %p OF TYPE %s WITH TARGET %p\n",
    //        (void*)n, der_types_str[n->astn_type.derived.type], (void*)n->astn_type.derived.target);
    return n;
}

/*
 * allocate decl type node
 */
astn *decl_alloc(astn *specs, astn *type, astn *init, YYLTYPE context) {
    astn *n=astn_alloc(ASTN_DECL);
    n->astn_decl.specs=specs;
    n->astn_decl.type=type;
    n->astn_decl.context=context;
    n->astn_decl.init = init;
    return n;
}

/*
 * allocate strunion variant of astn_typespec
 */
astn *strunion_alloc(struct st_entry* symbol) {
    astn *n=astn_alloc(ASTN_TYPESPEC);
    n->astn_typespec.is_tagtype = true;
    n->astn_typespec.symbol = symbol;
    n->astn_typespec.next = NULL;
    return n;
}

astn *fndef_alloc(astn* decl, astn* param_list, symtab* scope) {
    astn *n=astn_alloc(ASTN_FNDEF);
    n->astn_fndef.decl = decl;
    n->astn_fndef.param_list = param_list;
    n->astn_fndef.scope = scope;
    return n;
}

astn *declrec_alloc(st_entry* e, astn* init) {
    astn *n=astn_alloc(ASTN_DECLREC);
    n->astn_declrec.e = e;
    n->astn_declrec.init = init;
    return n;
}

astn *symptr_alloc(st_entry* e) {
    astn *n=astn_alloc(ASTN_SYMPTR);
    n->astn_symptr.e = e;
    return n;
}

astn *ifelse_alloc(astn *cond_s, astn *then_s, astn *else_s) {
    astn *n=astn_alloc(ASTN_IFELSE);
    n->astn_ifelse.condition_s = cond_s;
    n->astn_ifelse.then_s = then_s;
    n->astn_ifelse.else_s = else_s;
    return n;
}

astn *whileloop_alloc(astn* cond_s, astn* body_s, bool is_dowhile) {
    astn *n=astn_alloc(ASTN_WHILELOOP);
    n->astn_whileloop.is_dowhile = is_dowhile;
    n->astn_whileloop.condition = cond_s;
    n->astn_whileloop.body = body_s;
    return n;
}

astn *forloop_alloc(astn *init, astn* condition, astn* oneach, astn* body) {
    astn *n=astn_alloc(ASTN_FORLOOP);
    n->astn_forloop.init = init;
    n->astn_forloop.condition = condition;
    n->astn_forloop.oneach = oneach;
    n->astn_forloop.body = body;
    return n;
}

// this shouldn't be here >:(
astn *do_decl(astn *decl) {
    astn *n = NULL;
    if (decl->type == ASTN_DECL) {
        n = declrec_alloc(begin_st_entry(decl, NS_MISC, decl->astn_decl.context), decl->astn_decl.init);
        st_reserve_stack(n->astn_declrec.e);
    }
    return n;
}

/*
 * set the final derived.target of a chain of derived type nodes, eg:
 * (ptr to)->(array of)->...->target
 */
void set_dtypechain_target(astn *top, astn *target) {
    while (top->astn_type.derived.target) {
        top = top->astn_type.derived.target;
    }
    //ffprintf(stderr, stderr, "setting target to %p, I arrived at %p\n", (void*)target, (void*)top);
    top->astn_type.derived.target = target;
}

/*
 * same as above, but replace the existing final target, not append past it
 */
void reset_dtypechain_target(astn *top, astn *target) {
    astn* last = top;
    while (top->type == ASTN_TYPE && top->astn_type.derived.target) {
        last = top;
        top = top->astn_type.derived.target;
    }
    //ffprintf(stderr, stderr, "resetting target to %p, I arrived at %p\n\n", (void*)target, (void*)last);
    last->astn_type.derived.target = target;
}

/*
 * takes the final node of the parent (the IDENT), makes it the final node of the child,
 * and connects the child to the parent
 */
void merge_dtypechains(astn *parent, astn *child) {
    set_dtypechain_target(child, get_dtypechain_target(parent));
    reset_dtypechain_target(parent, child);
}

/*
 * get last target of chain of derived types
 */
astn* get_dtypechain_target(astn* top) {
    while (top->type == ASTN_TYPE && top->astn_type.derived.target) {
        top = top->astn_type.derived.target;
    }
    return top;
}
