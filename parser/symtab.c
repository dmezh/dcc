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

static void st_check_linkage(st_entry* e);
void st_dump_single_given(symtab* s);

// symtab for this translation unit
symtab root_symtab = {
    .scope_type = SCOPE_FILE,
    .parent     = NULL,
    .first      = NULL,
    .last       = NULL,
    .context    = {0} // must set with %initial-action
};

symtab *current_scope = &root_symtab;

// currently doesn't support actually defining already-declared functions
st_entry *st_define_function(astn* fndef, astn* block, YYLTYPE openbrace_context) {
    astn* decl_type = get_dtypechain_target(fndef->astn_decl.type);

    if (decl_type->type != ASTN_FNDEF) {
        fprintf(stderr, "Error near %s:%d: invalid declaration\n", openbrace_context.filename, openbrace_context.lineno);
        exit(-5);
    }

    struct astn_fndef fd = decl_type->astn_fndef;

    symtab *fn_scope = current_scope;
    st_pop_scope();

    const char* name = fd.decl->astn_ident.ident;
    st_entry *n = st_lookup_ns(name, NS_MISC);
    if (n && n->entry_type == STE_FN && n->fn_defined) {
        if (!block) return n; // ignore redecl
        fprintf(stderr, "Error: attempted redefinition of symbolss %s\n", name);
        exit(-5);
    }

    // following actions leak an astn_fndef
    if (fndef->astn_decl.type->type == ASTN_TYPE) // necessarily a derived type
        reset_dtypechain_target(fndef->astn_decl.type, fd.decl);
    else
        fndef->astn_decl.type = fndef->astn_decl.type->astn_fndef.decl;

    st_entry *fn = n ? n : begin_st_entry(fndef, NS_MISC, fndef->astn_decl.context);

    //printf("back from install honey\n");
    astn* p = fd.param_list;
    while (block && p) {
        if (list_data(p)->type != ASTN_DECLREC) {
            st_error("function parameter name omitted near %s:%d\n", fn_scope->context.filename, fn_scope->context.lineno);
        }
        p = list_next(p);
    }

    fn->body = block;
    fn->param_list = fd.param_list;
    fn->entry_type = STE_FN;
    fn->fn_scope = fn_scope;
    fn->fn_scope->parent_func = fn;
    //fn->fn_scope->stack_total = 0;
    fn->fn_scope->context = openbrace_context; // kludge up from grammar
    fn->storspec = SS_NONE;
    fn->def_context = openbrace_context; // this probably isn't consistent with structs, whatever
    fn->fn_defined = true;
    return fn;
}

/*
 *  Declare (optionally permissively) a struct in the current scope without defining.
 */
st_entry *st_declare_struct(const char* ident, bool strict, YYLTYPE context) {
    st_entry *n = st_lookup(ident, NS_TAGS);
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
        new->entry_type = STE_STRUNION_DEF;
        new->is_union = false;
        new->decl_context = context;
        new->linkage = L_NONE;
        new->storspec = SS_NONE;
        // get up to the closest scope in the stack that's not a mini-scope
        symtab* save = current_scope;
        while (current_scope == SCOPE_MINI)
            st_pop_scope();
        new->scope = current_scope;
        current_scope = save; // restore scope stack
        st_insert_given(new);
        return new;
    }
}

/*
 *  Declare and define a struct in the current scope.
 *
 *  Note: changes would need to be made to support unnamed structs.
 */
st_entry* st_define_struct(const char *ident, astn *decl_list,
                           YYLTYPE name_context, YYLTYPE closebrace_context, YYLTYPE openbrace_context) {
    st_entry *strunion;
    strunion = st_declare_struct(ident, true, name_context); // strict bc we're about to define!

    //printf("creating mini at %s:%d\n", openbrace_context.filename, openbrace_context.lineno);
    st_new_scope(SCOPE_MINI, openbrace_context);
    strunion->members = current_scope; // mini-scope
    st_entry *member;
    while (decl_list) {
        member = begin_st_entry(list_data(decl_list), NS_MEMBERS, list_data(decl_list)->astn_decl.context);
        member->linkage = L_NONE;
        member->storspec = SS_NONE;
        decl_list = list_next(decl_list);
    }
    strunion->def_context = closebrace_context;
    st_pop_scope();
    return strunion;
}

void st_make_union() {

}

// ret function scope if in function, else NULL
symtab* st_parent_function() {
    symtab* cur = current_scope;
    while (cur->scope_type == SCOPE_BLOCK)
        cur = cur->parent;
    if (cur->scope_type == SCOPE_FUNCTION)
        return cur;
    else
        return NULL;
}

// kludge up the linkage and storage specs for globals
static void st_check_linkage(st_entry* e) {
    if (e->scope == &root_symtab) {
        if (e->storspec == SS_UNDEF) { // plain declaration in global scope
            if (e->type->astn_type.is_const)// top level type
                e->linkage = L_INTERNAL;
            else
                e->linkage = L_EXTERNAL;
            e->storspec = SS_STATIC;
        } else if (e->storspec == SS_EXTERN) { // extern declaration in global scope
            e->linkage = L_EXTERNAL; // why does this make sense?
            e->storspec = SS_EXTERN;
        } else if (e->storspec == SS_STATIC) {
            e->linkage = L_INTERNAL;
            e->storspec = SS_STATIC;
        } else {
            st_error("Only 'static' and 'extern' are valid storage/linkage specifiers for top-level declarations!\n");
        }
    }
}

void st_reserve_stack(st_entry* e) {
    if (e->storspec == SS_AUTO) {
        int size;
        if (e->type->astn_type.is_derived && e->type->astn_type.derived.type == t_ARRAY) { // only diff size for arrays
            size = get_sizeof(e->type);
            //fprintf(stderr, "got arr size %d\n", size);
        } else size = 4;
        symtab *f = st_parent_function();
        if (e->is_param) {
            f->param_stack_total -= size;
            e->stack_offset = f->param_stack_total;
        } else {
            f->stack_total += size;
            e->stack_offset = f->stack_total;
            //printf("inres stack total is now %d\n", f->stack_total);
        }
    }
}


/*
 *  Synthesize a new st_entry, qualify and specify it, and attempt to install it
 *  into the current scope. Sets entry_type to STE_VAR by default.
 * 
 *  TODO: decl is not yet a list
 * 
 *  Note: It would introduce safety at compile-time to make the parameter an astn_decl,
 *  but I am taking the route of preferring all interfaces to the parser be plain astn* 's.
 */
st_entry* begin_st_entry(astn *decl, enum namespaces ns, YYLTYPE context) {
    if (decl->type != ASTN_DECL)
        die("Invalid astn for begin_st_entry() (astn_decl was expected)");

    astn *spec = decl->astn_decl.specs;
    astn *decl_list = decl->astn_decl.type;

    st_entry *new;
    if (decl->astn_decl.type->type == ASTN_FNDEF) { // GIGA kludge for function declarations
        st_entry *newfn = st_define_function(decl, NULL, context);
        newfn->fn_defined = false;
        newfn->def_context = (YYLTYPE){NULL, 0}; // it's not defined
        return newfn;
    }

    new = stentry_alloc(get_dtypechain_target(decl_list)->astn_ident.ident);
    new->ns = ns;

    struct astn_type *t = &new->type->astn_type;
    new->storspec = describe_type(spec, t);

    new->decl_context = context;
    new->scope = current_scope;
    new->entry_type = STE_VAR; // default; override from caller when needed!
    symtab* f = st_parent_function();
    if ((f && !new->storspec) || new->storspec == SS_AUTO) {
        new->storspec = SS_AUTO;
        //st_entry* func = st_parent_function()->parent_func;
        // storage alloc
    }
//    st_check_tentative(new);
    st_check_linkage(new);

    reset_dtypechain_target(decl_list, new->type); // end of chain is now the type instead of ident
    if (decl_list->type == ASTN_TYPE && decl_list->astn_type.is_derived) { // ALLOCATE HERE?
        new->type = decl_list; // because otherwise it's just an IDENT
    }

    if (!st_insert_given(new)) {
        if (new->scope == &root_symtab) {
            st_entry* prev = st_lookup_ns(new->ident, new->ns);
            // is previous extern? if so, completely replace it.
            // this logic and behavior is total shit, please fix this some day
            // this whole problem can only be described as disgusting
            // #1 the standard (6.9.2) is nearly incomprehensible on this point
            // #2 it seems like that rule is not always applied evenly across compilers.
            // H&S 4.8.5 was extremely useful on this. I'll try to follow a C++ -like model,
            // in which the only tentative definition is one that's 'extern', and those cannot have initializers.
            if (prev->storspec == SS_EXTERN && new->storspec != SS_EXTERN) {
                st_entry *next = prev->next;
                *prev = *new;
                prev->next = next;
                return prev;
            } else if (prev->storspec == SS_STATIC && new->storspec == SS_EXTERN) {
                return prev;
            } else if (prev->storspec == SS_EXTERN && new->storspec == SS_EXTERN) {
                return prev;
            }
        }
        st_error("attempted redeclaration of symbol %s\n", new->ident);
    }
    return new;
}

/* 
 *  Just allocate; we're not checking any kind of context for redeclarations, etc
 */
st_entry* stentry_alloc(const char *ident) {
    st_entry *n = safe_calloc(1, sizeof(st_entry));
    n->type = astn_alloc(ASTN_TYPE);
    n->ident = ident;
    return n;
}

/*
 *  Look up the symbol and return it if found, NULL otherwise
 *  // I'm not convinced this is all that's needed yet
 */
st_entry* st_lookup(const char* ident, enum namespaces ns) {
    symtab *cur = current_scope;
    st_entry* match;
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
st_entry* st_lookup_ns(const char* ident, enum namespaces ns) {
    return st_lookup_fq(ident, current_scope, ns);
}

/*
 *  Fully-qualified lookup; give me symtab to check and namespace
 */
st_entry* st_lookup_fq(const char* ident, const symtab* s, enum namespaces ns) {
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
 *  Create new scope/symtab and set it as current
 */
void st_new_scope(enum scope_types scope_type, YYLTYPE context) {
    symtab *new = safe_malloc(sizeof(symtab));
    *new = (symtab){
        .scope_type = scope_type,
        .stack_total= 4, // crime
        .param_stack_total = -4, // crime
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
        free(target++);
    }
}
