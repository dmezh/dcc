/*
 * symtab.c
 *
 * This file contains the interfaces for the symbol table.
 * It also maintains a static reference to the current scope in the scope stack.
 */

#include "symtab.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "types.h"
#include "util.h"

// symtab for this translation unit
symtab root_symtab = {
    .scope_type = SCOPE_FILE,
    .parent     = NULL,
    .first      = NULL,
    .last       = NULL
};
symtab *current_scope = &root_symtab;


/*
 * Synthesize a new st_entry, qualify and specify it, and attempt to install it
 * into the current scope.
 * 
 * TODO: decl_list is not yet a list
 */
void begin_st_entry(astn *spec, astn *decl_list) {
    // the below will need revising for lists, and also some error checking
    st_entry *new = stentry_alloc(get_dtypechain_target(decl_list)->astn_ident.ident);

    struct astn_type *t = &new->type->astn_type;
    new->storspec = describe_type(spec, t);
    reset_dtypechain_target(decl_list, new->type); // end of chain is now the type instead of ident
    if (decl_list->type == ASTN_TYPE && decl_list->astn_type.is_derived) {
        new->type = decl_list; // because otherwise it's just an IDENT
    }

    printf("Installing symbol <%s> into symbol table, storage class <%s>, with below type:\n", \
            new->ident, storspec_str[new->storspec]);
    print_ast(new->type);

    if (!st_insert_given(new)) {
        st_error("attempted redeclaration of symbol %s\n", new->ident);
    }
}

/* 
 * Just allocate; we're not checking any kind of context for redeclarations, etc
 */
st_entry* stentry_alloc(char *ident) {
    st_entry *n = safe_calloc(1, sizeof(st_entry));
    n->type = astn_alloc(ASTN_TYPE);
    n->ident = ident;
    return n;
}

/*
 *  Look up the symbol and return it if found, NULL otherwise
 *  //todo: walk the scope stack, not just current scope (when needed)
 *  //todo: namespace stuff (a.k.a properly check conflicts)
 */
st_entry* st_lookup(const char* ident) {
    st_entry* cur = current_scope->first;
    while (cur) {   // works fine for first being NULL / empty symtab
        if (!strcmp(ident, cur->ident))
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/*
 *  Insert given symbol into current scope
 *  return:
 *          true - success
 *          false - ident already in symtab
 */

bool st_insert_given(st_entry *new) {
    if (st_lookup(new->ident)) return false;
    new->next = NULL;
    if (!current_scope->first) { // currently-empty symtab
        current_scope->first = new;
        current_scope->last = new;
    } else {
        current_scope->last->next = new; // previous past will point to new last
        current_scope->last = new;
    }
    return true;
}

/*
 *  Insert new symbol in current scope
 *  return:
 *          true  - success
 *          false - ident already in current symtab
 *  //todo: set type, namespace, initializer stuff
 */
bool st_insert(char* ident) {
    st_entry *new = safe_malloc(sizeof(st_entry));
    new->ident = ident;
    return st_insert_given(new);
}

/*
 *  Create new scope/symtab and set it as current
 */
void st_new_scope(enum scope_types scope_type) {
    symtab *new = safe_malloc(sizeof(symtab));
    *new = (symtab){
        .scope_type = scope_type,
        .parent     = current_scope,
        .first      = NULL,
        .last       = NULL
    };
    current_scope = new;
}

/*
 *  Leave the current scope (return to parent)
 */
void st_pop_scope() {
    if (!current_scope->parent) {
        if (current_scope == &root_symtab) {
            die("Attempted to pop root scope");
        } else {
            die("Scope has NULL parent!");
        }
    }
    current_scope = current_scope->parent;
}

/*
 *  Destroy symbol table, freeing all st_entry, but not their .type or .ident members
 *  If root scope, just free the entries (since symtab itself is static)
 */
void st_destroy(symtab* target) {
    // free all the entries first
    st_entry *next = target->first->next;
    for (st_entry *e=target->first; e!=NULL; e=next) {
        next = e->next;
        free(e);
    }
    if (target == &root_symtab) { // in case you accidentally reuse root_symtab after this
        target->first = NULL;     //                            but seriously please don't
        target->last = NULL;
    } else {
        free(target);
    }
}

/*
 *  Dump a single symbol table
 */
void st_dump_single() {
    printf("Dumping symbol table!\n");
    st_entry* cur = current_scope->first;
    while (cur) {
        printf("ST_ENTRY with ident %s\n", cur->ident);
        cur = cur->next;
    }
}

// currently only a single scope!!
