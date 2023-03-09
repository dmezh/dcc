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

#include "ir_defs.h"
#include "lexer.h"
#include "symtab.h"
#include "util.h"

/*
 * Allocate single astn safely.
 * The entire compiler relies on the astn type being set, so we must do that.
 */
astn astn_alloc(enum astn_types type) {
    astn n = safe_calloc(1, sizeof(struct astn));
    n->type = type;
    n->context = context;
    return n;
}

/*
 * Return string version of astn's kind (e.g. ASTN_DECL)
 */
const char *astn_kind_str(astn a) {
    if (!a)
        die("Passed null astn to astn_kind_str!");

    if (a->type <= ASTN_KIND_UNDEF || a->type >= ASTN_KIND_MAX) {
        eprintf("Invalid astn kind: %d\n", a->type);
        die("Passed invalid astn kind to astn_kind_str!");
    }

    static const char *astn_kinds_str[] = {
        FOREACH_ASTN_KIND(GENERATE_STRING)
    };

    return astn_kinds_str[a->type];
}

/*
 * Allocate basic signed constant.
 */
astn simple_constant_alloc(int num) {
    astn n=astn_alloc(ASTN_NUM);
    n->Num.number.integer = num;
    n->Num.number.is_signed = true;
    n->Num.number.aux_type = s_INT;
    return n;
}

/*
 * allocate complex assignment (*=, /=, etc) - 6.5.16
 */
astn cassign_alloc(int op, astn left, astn right) {
    astn n=astn_alloc(ASTN_CASSIGN);
    n->Cassign.left=left;
    n->Cassign.right=right;
    n->Cassign.op=op;
    return n;
}

/*
 * allocate binary operation
 */
astn binop_alloc(int op, astn left, astn right) {
    astn n=astn_alloc(ASTN_BINOP);
    n->Binop.op=op;
    n->Binop.left=left;
    n->Binop.right=right;
    return n;
}

/*
 * allocate unary operation
 */
astn unop_alloc(int op, astn target) {
    astn n=astn_alloc(ASTN_UNOP);
    n->Unop.op=op;
    n->Unop.target=target;
    return n;
}

/*
 * allocate a list node
 */
astn list_alloc(astn me) {
    astn l=astn_alloc(ASTN_LIST);
    l->List.me=me;
    l->List.next=NULL;
    return l;
}

/*
 * allocate a list node and append it to end of existing list (from head)
 * return: ptr to new node
 */
//              (arg to add)(head of ll)
astn list_append(astn new, astn head) {
    ast_check(head, ASTN_LIST, "Expected ASTN_LIST node to append to.");

    astn n=list_alloc(new);
    while (head->List.next) head=head->List.next;
    head->List.next = n;
    return n;
}

/*
 *  get next node of list
 */
astn list_next(astn cur) {
    ast_check(cur, ASTN_LIST, "Expected ASTN_LIST node.");

    return cur->List.next;
}

/*
 *  get current data element
 */
astn list_data(astn n) {
    ast_check(n, ASTN_LIST, "Expected ASTN_LIST node.");

    return n->List.me;
}

// DSA shit
// https://www.geeksforgeeks.org/reverse-a-linked-list/
void list_reverse(astn *l) {
    astn prev=NULL, current=*l, next=NULL;
    while (current) {
        ast_check(current, ASTN_LIST, "Expected ASTN_LIST node.");

        next = list_next(current);
        current->List.next = prev;
        prev = current;
        current = next;
    }
    *l = prev;
}

/*
 * return length of AST list starting at head
 */
unsigned list_measure(astn head) {
    ast_check(head, ASTN_LIST, "Expected ASTN_LIST node.");

    int c = 0;
    while ((head=head->List.next)) {
        c++;
    }
    return c + 1;
}

/*
 * allocate type specifier node
 */
astn typespec_alloc(enum typespec spec) {
    astn n=astn_alloc(ASTN_TYPESPEC);
    n->Typespec.spec = spec;
    n->Typespec.next = NULL;
    //eprintf("ALLOCATED TSPEC\n");
    return n;
}

/*
 * allocate type qualifier node
 */
astn typequal_alloc(enum typequal qual) {
    astn n=astn_alloc(ASTN_TYPEQUAL);
    n->Typequal.qual = qual;
    n->Typespec.next = NULL;
    return n;
}

/*
 * allocate storage specifier node
 */
astn storspec_alloc(enum storspec spec) {
    astn n=astn_alloc(ASTN_STORSPEC);
    n->Storspec.spec = spec;
    n->Storspec.next = NULL;
    return n;
}

/*
 * allocate derived type node
 */
astn dtype_alloc(astn target, enum der_types type) {
    astn n=astn_alloc(ASTN_TYPE);
    n->Type.is_derived = true;
    n->Type.derived.type = type;
    n->Type.derived.target = target;
    //eprintf("ALLOCATED DTYPE %p OF TYPE %s WITH TARGET %p\n",
    //        (void*)n, der_types_str[n->Type.derived.type], (void*)n->Type.derived.target);
    return n;
}

/*
 * allocate decl type node
 */
astn decl_alloc(astn specs, astn type, astn init, YYLTYPE context) {
    astn n=astn_alloc(ASTN_DECL);
    n->Decl.specs=specs;
    n->Decl.type=type;
    n->Decl.init = init;
    n->context=context;
    return n;
}

/*
 * allocate strunion variant of astn_typespec
 */
astn strunion_alloc(sym symbol) {
    astn n=astn_alloc(ASTN_TYPESPEC);
    n->Typespec.is_tagtype = true;
    n->Typespec.symbol = symbol;
    n->Typespec.next = NULL;
    return n;
}

astn fndef_alloc(astn decl, astn param_list, symtab* scope) {
    astn n=astn_alloc(ASTN_FNDEF);
    n->Fndef.decl = decl;
    n->Fndef.param_list = param_list;
    n->Fndef.scope = scope;
    return n;
}

astn declrec_alloc(sym e, astn init) {
    astn n=astn_alloc(ASTN_DECLREC);
    n->Declrec.e = e;
    n->Declrec.init = init;
    return n;
}

astn symptr_alloc(sym e) {
    astn n=astn_alloc(ASTN_SYMPTR);
    n->Symptr.e = e;
    return n;
}

astn ifelse_alloc(astn cond_s, astn then_s, astn else_s) {
    astn n=astn_alloc(ASTN_IFELSE);
    n->Ifelse.condition_s = cond_s;
    n->Ifelse.then_s = then_s;
    n->Ifelse.else_s = else_s;
    return n;
}

astn whileloop_alloc(astn cond_s, astn body_s, bool is_dowhile) {
    astn n=astn_alloc(ASTN_WHILELOOP);
    n->Whileloop.is_dowhile = is_dowhile;
    n->Whileloop.condition = cond_s;
    n->Whileloop.body = body_s;
    return n;
}

astn forloop_alloc(astn init, astn condition, astn oneach, astn body) {
    astn n=astn_alloc(ASTN_FORLOOP);
    n->Forloop.init = init;
    n->Forloop.condition = condition;
    n->Forloop.oneach = oneach;
    n->Forloop.body = body;
    return n;
}

// this shouldn't be here >:(
astn do_decl(astn decl) {
    if (!decl) {
        die("Don't call do_decl with a useless declaration, handle before calling.");
    }
    ast_check(decl, ASTN_DECL, "Expected ASTN_DECL for do_decl");
    astn n = declrec_alloc(begin_st_entry(decl, NS_MISC, decl->context), decl->Decl.init);

    symtab *p = st_parent_function();

    if (!p)
        return n;

    if (!p->all_syms)
        p->all_syms = list_alloc(n);
    else
        list_append(n, p->all_syms);

    return n;
}

/*
 * set the final derived.target of a chain of derived type nodes, eg:
 * (ptr to)->(array of)->...->target
 */
void set_dtypechain_target(astn top, astn target) {
    while (top->Type.derived.target) {
        top = top->Type.derived.target;
    }
    //eprintf("setting target to %p, I arrived at %p\n", (void*)target, (void*)top);
    top->Type.derived.target = target;
}

/*
 * same as above, but replace the existing final target, not append past it
 */
void reset_dtypechain_target(astn top, astn target) {
    astn last = top;
    while (top->type == ASTN_TYPE && top->Type.derived.target) {
        last = top;
        top = top->Type.derived.target;
    }
    //eprintf("resetting target to %p, I arrived at %p\n\n", (void*)target, (void*)last);
    last->Type.derived.target = target;
}

/*
 * takes the final node of the parent (the IDENT), makes it the final node of the child,
 * and connects the child to the parent
 */
void merge_dtypechains(astn parent, astn child) {
    set_dtypechain_target(child, get_dtypechain_target(parent));
    reset_dtypechain_target(parent, child);
}

/*
 * Get last derived link in chain of derived types
 */
astn get_dtypechain_last_link(astn top) {
    while (top->type == ASTN_TYPE &&
           top->Type.derived.target &&
           top->Type.derived.target->type == ASTN_TYPE) {
        top = top->Type.derived.target;
    }
    return top;
}

/*
 * Get final target of chain of derived types
 */
astn get_dtypechain_target(astn top) {
    while (top->type == ASTN_TYPE && top->Type.derived.target) {
        top = top->Type.derived.target;
    }
    return top;
}

/*
 * Get ident at end of chain of derived types
 *
 * Not all dtypechains will have an ident at the end, so only call this if you
 * know and require that the ident is there.
 */
const char *get_dtypechain_ident(astn d) {
    astn i = get_dtypechain_target(d);
    ast_check(i, ASTN_IDENT, "Expected ident at end of dtypechain.");
    return i->Ident.ident;
}

/*
 * Allocate Qtemp astn
 */
astn qtemp_alloc(int tempno, astn qtype) {
    astn q = astn_alloc(ASTN_QTEMP);
    q->Qtemp.tempno = tempno;
    q->Qtemp.qtype = qtype;
    return q;
}

/*
 * Allocate Qtype astn, with no derived type by default
 */
astn qtype_alloc(ir_type_E t) {
    astn q = astn_alloc(ASTN_QTYPE);
    q->Qtype.ir_type = t;
    q->Qtype.derived_type = NULL;
    return q;
}
