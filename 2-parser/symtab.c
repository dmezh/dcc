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

const char* st_entry_types_str[] = {
    [STE_UNDEF] = "UNDEF!",
    [STE_VAR] = "var",
    [STE_STRUNION_DEF] = "s_u",
    [STE_FN] = "fnc"
};

const char* linkage_str[] = {
    [L_UNDEF] = "UNDEF!",
    [L_NONE] = "NONE",
    [L_INTERNAL] = "INTERNAL",
    [L_EXTERNAL] = "EXTERNAL"
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

// currently doesn't support actually defining already-declared functions
st_entry *st_define_function(astn* fndef, astn* block, YYLTYPE openbrace_context) {
    symtab *fn_scope = current_scope;
    st_pop_scope();
    struct astn_fndef f2 = (get_dtypechain_target(fndef->astn_decl.type))->astn_fndef;
    char* name = f2.decl->astn_ident.ident;
    st_entry *n = st_lookup_ns(name, NS_MISC);
    if (n) {
        fprintf(stderr, "Error: attempted redefinition of symbol %s\n", name);
        exit(-5);
    } else {
        // following actions leak an astn_fndef
        if (fndef->astn_decl.type->type == ASTN_TYPE) // necessarily a derived type
            reset_dtypechain_target(fndef->astn_decl.type, f2.decl);
        else
            fndef->astn_decl.type = fndef->astn_decl.type->astn_fndef.decl;

        st_entry *fn = begin_st_entry(fndef, NS_MISC, fndef->astn_decl.context);
        //printf("back from install honey\n");
        fn->body = block;
        fn->param_list = f2.param_list;
        fn->entry_type = STE_FN;
        fn->fn_scope = fn_scope;
        fn->fn_scope->context = openbrace_context; // kludge up from grammar
        fn->storspec = SS_NONE;
        fn->def_context = openbrace_context; // this probably isn't consistent with structs, whatever
        fn->fn_defined = true;
        return fn;
    }
}

/*
 *  Declare (optionally permissively) a struct in the current scope without defining.
 */
st_entry *st_declare_struct(char* ident, bool strict, YYLTYPE context) {
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
st_entry* st_define_struct(char *ident, astn *decl_list,
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
    if (current_scope->scope_type == SCOPE_FUNCTION && !new->storspec)
        new->storspec = SS_AUTO;
//    st_check_tentative(new);
    st_check_linkage(new);

    reset_dtypechain_target(decl_list, new->type); // end of chain is now the type instead of ident
    if (decl_list->type == ASTN_TYPE && decl_list->astn_type.is_derived) {
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
st_entry* stentry_alloc(char *ident) {
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

void st_dump_single_given(symtab* s) {
    //printf("Dumping symbol table!\n");
    st_entry* cur = s->first;
    while (cur) {
        st_dump_entry(cur);
        cur = cur->next;
    }
}

//
//
//
//

/*
 *  Dump a single symbol table (the current_scope) with basic list
 */
void st_dump_single() {
    st_dump_single_given(current_scope);
}

/*
 *  Dump a single st_entry with one-line info.
 */
void st_dump_entry(const st_entry* e) {
    if (!e) return;
    printf("%s<%s> <ns:%s> <stor:%s%s> <scope:%s @ %s:%d>",
            st_entry_types_str[e->entry_type],
            e->ident,
            namespaces_str[e->ns],
            storspec_str[e->storspec],
            e->is_param ? "*" : "",
            scope_types_str[e->scope->scope_type],
            e->scope->context.filename,
            e->scope->context.lineno
    );
    if (e->scope == &root_symtab && e->linkage && e->linkage != L_NONE) {
        printf(" <linkage:%s>", linkage_str[e->linkage]);
    }
    if (e->decl_context.filename) {
        printf(" <decl %s:%d>", e->decl_context.filename, e->decl_context.lineno);
    }
    if ((e->ns == NS_TAGS && !e->members) || (e->entry_type == STE_FN && !e->fn_defined)) {
        printf(" (FWD-DECL)");
    }
    if (e->def_context.filename) {
        printf(" <def %s:%d>", e->def_context.filename, e->def_context.lineno);
    }
    printf("\n");
}

/*
 *  Output in-depth info for given st_entry based on entry_type.
 *  Call st_dump_entry() first if you want to get the symbol info line.
 */
void st_examine_given(st_entry* e) {
    if (e->entry_type == STE_FN) {
        printf("FUNCTION RETURNING:\n");
        print_ast(e->type);
        if (e->fn_defined) {
            if (e->param_list) {
                printf("AND TAKING PARAMS:\n");
                print_ast(e->param_list);
            } else {
                printf("TAKING NO PARAMETERS\n");
            }
            printf("DUMPING FUNCTION SYMTAB:\n");
            st_dump_single_given(e->fn_scope);
            printf("DUMPING FUNCTION AST:\n");
            print_ast(e->body);
        }
    }
    if (e->entry_type != STE_FN && (e->ns == NS_MEMBERS || e->ns == NS_MISC)) {
        print_ast(e->type);
    }
    if (e->entry_type == STE_STRUNION_DEF && e->members) {
        printf("Dumping tag-type mini scope of tag <%s>!\n", e->ident);
        st_entry* cur = e->members->first;
        while (cur) {
            st_dump_entry(cur);
            st_examine_given(cur);
            cur = cur->next;
        }
    }
}

/*
 *  Search for an ident across all the namespaces and output st_entry info.
 */
void st_examine(char* ident) {
    st_entry *e;
    int count = 0;       // yeah, weird way of getting the enum size
    for (unsigned i=0; i<sizeof(namespaces_str)/sizeof(char*); i++) {
        if ((e = st_lookup(ident, i))) {
            count++;
            printf("> found <%s>, here is the entry (and type if applicable):\n", ident);
            st_dump_entry(e);
            st_examine_given(e);
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
    st_entry *e = st_lookup(tag, NS_TAGS);
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
    return;
}

// kind of fake at the moment
void st_dump_recursive() {
    printf("_- symtab dump for translation unit: -_\n");
    st_entry *e = current_scope->first;
    while (e) {
        st_dump_entry(e);
        st_examine_given(e);
        printf("\n");
        e = e->next;
    }
}
