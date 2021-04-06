#include "symtab.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"

// symtab for this translation unit
symtab root_symtab = {
    .scope_type = SCOPE_FILE,
    .parent     = NULL,
    .first      = NULL,
    .last       = NULL
};

symtab* current_scope = &root_symtab;

/*
 *  Look up the symbol and return it if found, NULL otherwise
 *  //todo: walk the scope stack, not just current scope
 *  //todo: namespace stuff
 */
st_entry* st_lookup(char* ident) {
    st_entry* cur = current_scope->first;
    while (cur) {   // works fine for first being NULL / empty symtab
        if (!strcmp(ident, cur->ident))
            return cur;
        cur = cur->next;
    }
    return NULL;
}

/*
 *  Insert new symbol in current scope
 *  return:
 *          true  - success
 *          false - ident already in current symtab
 */
bool st_insert(char* ident) {
    if (st_lookup(ident)) return false;
    st_entry *new = safe_malloc(sizeof(st_entry));
    new->ident = ident;
    if (!current_scope->first) { // currently-empty symtab
        current_scope->first = new;
        current_scope->first->next = NULL;
        current_scope->last = new;
        return true;
    }
    current_scope->last->next = new; // previous past will point to new last
    current_scope->last = new;
    return true;
}

/*
 *  Create new scope/symtab and set it as current
 */
void new_scope(enum scope_types scope_type) {
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
void pop_scope() {
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
 * Destroy symbol table, freeing all st_entry, but not their .type or .ident members
 */
void destroy_symtab(symtab* target) {
    // free all the entries first
    st_entry* next = target->first->next;
    for (st_entry *e=target->first; e!=NULL; e=next) {
        next = e->next;
        free(e);
    }
    free(target);
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
