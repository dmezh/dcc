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

#include "location.h"
#include "types.h"
#include "util.h"

const char* scope_types_str[] = {
    [SCOPE_MINI] = "MINI",
    [SCOPE_FILE] = "GLOBAL",
    [SCOPE_FUNCTION] = "FUNCTION",
    [SCOPE_BLOCK] = "BLOCK",
    [SCOPE_PROTOTYPE] = "PROTOTYPE"
};

const char* namespaces_str[] = {
    [NS_TAGS] = "TAGS",
    [NS_LABELS] = "LABELS",
    [NS_MEMBERS] = "MEMBERS",
    [NS_MISC] = "MISC"
};

// symtab for this translation unit
symtab root_symtab = {
    .scope_type = SCOPE_FILE,
    .parent     = NULL,
    .first      = NULL,
    .last       = NULL,
    .context    = {0} // must set with %initial-action
};
symtab *current_scope = &root_symtab;

/*
 *  Declare (optionally permissively) a struct in the current scope without defining.
 */
st_entry *st_declare_struct(char* ident, bool strict, YYLTYPE context) {
    st_entry *n = st_lookup_ns(ident, NS_TAGS);
    if (n) {
        if (strict && n->members) {
            fprintf(stderr, "Error: attempted redeclaration of complete tag");
            exit(-5);
        } else {
            return n; // "redeclared"
        }
    } else {
        st_entry *new = stentry_alloc(ident);
        new->ns = NS_TAGS;
        new->is_strunion_def = true;
        new->is_union = false;
        new->decl_context = context;
        new->scope = current_scope;
        st_insert_given(new);
        return new;
    }
}

/*
 *  Declare and define a struct in the current scope.
 */
st_entry* st_define_struct(char *ident, astn *decl_list, YYLTYPE context,  YYLTYPE openbrace_context) {
    st_entry *strunion;
    if (ident)
        strunion = st_declare_struct(ident, true, context); // strict bc we're about to define!
   // printf("creating mini at %s:%d\n", openbrace_context.filename, openbrace_context.lineno);
    st_new_scope(SCOPE_MINI, openbrace_context);
    strunion->members=current_scope; // mini-scope
    while (decl_list) {
        begin_st_entry(list_data(decl_list), NS_MEMBERS, list_data(decl_list)->astn_decl.context);
        decl_list = list_next(decl_list);
    }
    
    strunion->def_context=context;
    st_pop_scope();
    return strunion;
}

void st_make_union() {

}

/*
 *  Synthesize a new st_entry, qualify and specify it, and attempt to install it
 *  into the current scope.
 * 
 *  TODO: decl_list is not yet a list
 * 
 *  Note: It would introduce safety at compile-time to make the parameter an astn_decl,
 *  but I am taking the route of preferring all interfaces to the parser be plain astn* 's.
 */
void begin_st_entry(astn *decl, enum namespaces ns, YYLTYPE context) {
    if (decl->type != ASTN_DECL)
        die("Invalid astn for begin_st_entry() (astn_decl was expected)");

    astn *spec = decl->astn_decl.specs;
    astn *decl_list = decl->astn_decl.type;

    st_entry *new = stentry_alloc(get_dtypechain_target(decl_list)->astn_ident.ident);
    new->ns = ns;

    struct astn_type *t = &new->type->astn_type;
    new->storspec = describe_type(spec, t);
    new->decl_context = context;
    new->scope = current_scope;

    reset_dtypechain_target(decl_list, new->type); // end of chain is now the type instead of ident
    if (decl_list->type == ASTN_TYPE && decl_list->astn_type.is_derived) {
        new->type = decl_list; // because otherwise it's just an IDENT
    }

    if (!st_insert_given(new)) {
        st_error("attempted redeclaration of symbol %s\n", new->ident);
    }
}

/* 
 *  Just allocate; we're not checking any kind of context for redeclarations, etc
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
 *  Lookup in current_scope, with namespace
 */
st_entry* st_lookup_ns(const char* ident, enum namespaces ns) {
    return st_lookup_fq(ident, current_scope, ns);
}

/*
 *  Fully-qualified lookup; give me symtab to check and namespace
 */
st_entry* st_lookup_fq(const char* ident, symtab* s, enum namespaces ns) {
    st_entry* cur = s->first;
    while (cur) {
        if (cur->ns == ns && !strcmp(ident, cur->ident))
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
 *  Create new scope/symtab and set it as current
 */
void st_new_scope(enum scope_types scope_type, YYLTYPE context) {
    symtab *new = safe_malloc(sizeof(symtab));
    *new = (symtab){
        .scope_type = scope_type,
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
 *  Dump a single st_entry.
 */
void st_dump_entry(const st_entry* e) {
    if (!e) return;
    printf("sym<%s> <ns:%s> <scope:%s @ %s:%d>", e->ident, namespaces_str[e->ns], scope_types_str[e->scope->scope_type],
            e->scope->context.filename, e->scope->context.lineno);
    if (e->ns == NS_TAGS) {
        printf("%s", e->members ? " (DEFINED)" : " (FWD-DECL)");
    }
    if (e->decl_context.filename) {
        printf(" <decl %s:%d>", e->decl_context.filename, e->decl_context.lineno);
    }
    if (e->def_context.filename) {
        printf(" <def %s:%d>", e->def_context.filename, e->def_context.lineno);
    }
    printf("\n");
}

/*
 *  Dump a single symbol table (the current_scope)
 */
void st_dump_single() {
    printf("Dumping symbol table!\n");
    st_entry* cur = current_scope->first;
    while (cur) {
        st_dump_entry(cur);
        cur = cur->next;
    }
}

/*
 *  Dump a struct with members
 */
void st_dump_struct(st_entry* s) {
    printf("Dumping tag-type mini scope of tag <%s>!\n", s->ident);
    st_entry* cur = s->members->first;
    while (cur) {
        st_dump_entry(cur);
        cur = cur->next;
    }
}

/*
 *  Search for an ident across all the namespaces and output st_entry info.
 */
void st_examine(char* ident) {
    st_entry *e;
    int count = 0;
    for (unsigned i=0; i<sizeof(namespaces_str)/sizeof(char*); i++) {
        if ((e = st_lookup_ns(ident, i))) {
            count++;
            printf("> found <%s>, here is the entry (and type if member or misc):\n", ident);
            st_dump_entry(e);
            if (i == NS_MEMBERS || i == NS_MISC)
                print_ast(e->type);
            if (e->is_strunion_def && e->members) {
                st_dump_struct(e);
            }
        }
    }
    if (!count) {
        printf("> No matches found for <%s> using lookup from current scope (%p)\n", ident, (void*)current_scope);
    }
    printf("\n");
}

/*
 *  Search for a member of a tag and output st_entry info.
 */
void st_examine_member(char* tag, char* child) {
    st_entry *e = st_lookup_ns(tag, NS_TAGS);
    if (!e) {
        printf("> I was not able to find tag <%s>.\n\n", tag);
        return;
    }
    if (!e->members) {
        printf("> Tag <%s> is declared, but not defined:\n", tag);
        st_dump_entry(e);
        printf("\n");
        return;
    }
    st_entry *m = st_lookup_fq(child, e->members, NS_MEMBERS);
    if (!m) {
        printf("> I found tag <%s>, but not the member <%s>:\n", tag, child);
        st_dump_entry(e);
        printf("\n");
        return;
    }
    printf("> Found <%s> -> <%s>:\n", tag, child);
    st_dump_entry(m);
    print_ast(m->type);
    printf("\n");
    return;
}
