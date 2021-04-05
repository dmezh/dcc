#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "util.h"
#include "symtab.h"

// symtab for this translation unit
symtab root_symtab = {
    .scope_type = SCOPE_FILE,
    .parent     = NULL,
    .first      = NULL,
    .last       = NULL
};

symtab* current_scope = &root_symtab;

// look up the symbol and return it if found, NULL otherwise
// todo: walk the scope stack, not just current scope
st_entry* st_lookup(char* ident) {
    st_entry* cur = current_scope->first;
    while (cur) {   // works fine for first being NULL / empty symtab
        if (!strcmp(ident, cur->ident))
            return cur;
        cur = cur->next;
    }
    return NULL;
}

// true if successful, false if ident taken
bool st_insert(char* ident) {
    if (st_lookup(ident)) return false;
    st_entry* new = safe_malloc(sizeof(st_entry));
    new->ident = ident;
    
    if (!current_scope->first) { // currently-empty symtab
        current_scope->first = new;
        current_scope->last = new;
        return true;
    }
    current_scope->last->next = new; // previous past will point to new last
    current_scope->last = new;
    return true;
}

void st_dump_single() {
    printf("Dumping symbol table!\n");
    st_entry* cur = current_scope->first;
    while (cur) {
        printf("ST_ENTRY with ident %s\n", cur->ident);
        cur = cur->next;
    }
}

// currently only a single scope!!