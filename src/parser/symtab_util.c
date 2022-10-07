/*
 * symtab_util.c
 *
 * Basic symbol table and scope stack tools.
 */

#include "symtab_util.h"

#include <stdlib.h>
#include <string.h>

#include "symtab.h"
#include "util.h"

/* 
 *  Just allocate; we're not checking any kind of context for redeclarations, etc
 */
sym stentry_alloc(const char *ident) {
    sym n = safe_calloc(1, sizeof(st_entry));
    n->type = astn_alloc(ASTN_TYPE);
    n->ident = ident;
    return n;
}


/*
 *  Insert given symbol into current scope
 *  return:
 *          true - success
 *          false - ident already in symtab
 */
bool st_insert_given(sym new) {
    if (st_lookup_ns(new->ident, new->ns)) return false;
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
 *  Look up the symbol and return it if found, NULL otherwise
 *  // I'm not convinced this is all that's needed yet
 */
sym st_lookup(const char* ident, enum namespaces ns) {
    symtab *cur = current_scope;
    sym match;
    while (cur) {
        if ((match = st_lookup_fq(ident, cur, ns)))
            return match;
        else
            cur = cur->parent;
    }
    return NULL;
}


/*
 *  Lookup in current_scope, with namespace
 */
sym st_lookup_ns(const char* ident, enum namespaces ns) {
    return st_lookup_fq(ident, current_scope, ns);
}


/*
 *  Fully-qualified lookup; give me symtab to check and namespace
 */
sym st_lookup_fq(const char* ident, const symtab* s, enum namespaces ns) {
    sym cur = s->first;
    while (cur) {
        if (cur->ns == ns && !strcmp(ident, cur->ident))
            return cur;
        cur = cur->next;
    }
    return NULL;
}


/*
 *  Create new scope/symtab and set it as current
 */
void st_new_scope(enum scope_types scope_type, YYLTYPE context) {
    symtab *new = safe_malloc(sizeof(symtab));
    *new = (symtab){
        .scope_type = scope_type,
        .stack_total= 8, // crime
        .param_stack_total = -8, // crime
        .context    = context,
        .parent     = current_scope,
        .first      = NULL,
        .last       = NULL
    };
    current_scope = new;
}


/*
 *  Leave the current scope (return to parent)
 */
void st_pop_scope(void) {
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
    sym next = target->first->next;
    for (sym e=target->first; e!=NULL; e=next) {
        next = e->next;
        free(e);
    }
    if (target == &root_symtab) { // in case you accidentally reuse root_symtab after this
        target->first = NULL;     //                            but seriously please don't
        target->last = NULL;
    } else {
        free(target);
        free(target++);
    }
}

