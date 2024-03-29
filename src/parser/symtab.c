/*
 * symtab.c
 *
 * This file contains the interfaces for the symbol table.
 * It also maintains a static reference to the current scope in the scope stack.
 */

#include "symtab.h"

#include <stdbool.h>
#include <stdio.h>

#include "ast.h"
#include "location.h"
#include "symtab_util.h"
#include "types.h"
#include "util.h"

static void st_check_linkage(sym e);
static sym real_begin_st_entry(astn decl, enum namespaces ns, YYLTYPE context);

// symtab for this translation unit
symtab root_symtab = {
    .scope_type = SCOPE_FILE,
    .parent     = NULL,
    .first      = NULL,
    .last       = NULL,
    .context    = {0} // must set with %initial-action
};

symtab *current_scope = &root_symtab;

sym st_define_function(astn fndef, astn block, YYLTYPE context) {
    ast_check(fndef, ASTN_DECL, "Expected decl.");
    ast_check(block, ASTN_LIST, "Expected list for fn body.");

    // get the name
    const char *name = get_dtypechain_ident(fndef->Decl.type);

    // check if function has been declarated and/or defined
    sym fn = st_lookup(name, NS_MISC);
    if (fn) { // found existing entry
        if (fn->entry_type == STE_FN) { // entry is fn
            if (fn->fn_defined) { // fn defined
                st_error("Attempted redefinition of function %s near %s:%d\n", name, context.filename, context.lineno);
            }
        } else {
            st_error("Attempted redeclaration of symbol %s near %s:%d\n", name, context.filename, context.lineno);
        }
    }

    // check existing declaration's compatibility at this point
    fn = st_declare_function(fndef, context);

    // define the scope's context and function body AST
    fn->body = block;
    fn->fn_defined = true;
    fn->fn_scope->context = context;

    return fn;
}


sym st_declare_function(astn decl, YYLTYPE context) {
    ast_check(decl, ASTN_DECL, "Expected decl.");
    ast_check(decl->Decl.type, ASTN_TYPE,
              "Invalid non-type astn at top of function declaration type chain.");

    struct astn_type *decl_type = &decl->Decl.type->Type;

    if (decl_type->derived.type != t_FN) {
        st_error("Invalid declaration near %s:%d:\n", context.filename, context.lineno);
    }

    // get the name
    const char *name = get_dtypechain_ident(decl->Decl.type);

    // make new st_entry if needed
    sym fn = st_lookup_ns(name, NS_MISC);
    if (!fn) // check for compatibility with existing declaration here
        fn = real_begin_st_entry(decl, NS_MISC, decl->context);

    // check the parameter list for ellipses and missing names
    astn p = decl_type->derived.param_list;
    while (p) {
        if (list_data(p)->type == ASTN_ELLIPSIS) {
            fn->variadic = true;
        } else if (list_data(p)->type != ASTN_DECLREC) {
            YYLTYPE *scope_context = &decl_type->derived.scope->context;
            st_error("function parameter name omitted near %s:%d\n", scope_context->filename, scope_context->lineno);
        }
        p = list_next(p);
    }

    // set the rest of the st_entry fields we'll need
    fn->param_list = decl_type->derived.param_list;
    fn->entry_type = STE_FN;
    fn->fn_scope = decl_type->derived.scope; // we're destroying any existing scope, fix this for type-checking prototypes.
    fn->fn_scope->parent_func = fn;
    fn->storspec = SS_NONE;
    fn->def_context = context; // this probably isn't consistent with structs, whatever
    fn->fn_defined = false;

    return fn;
}


/*
 *  Declare (optionally permissively) a struct in the current scope without defining.
 */
sym st_declare_struct(const char* ident, bool strict, YYLTYPE context) {
    sym n = st_lookup(ident, NS_TAGS);
    if (n) {
        if (strict && n->members) {
            eprintf("Error: attempted redeclaration of complete tag");
            exit(-5);
        } else {
            return n; // "redeclared"
        }
    } else {
        sym new = stentry_alloc(ident);
        new->ns = NS_TAGS;
        new->entry_type = STE_STRUNION_DEF;
        new->is_union = false;
        new->decl_context = context;
        new->linkage = L_NONE;
        new->storspec = SS_NONE;
        new->scope = current_scope;
        // get up to the closest scope in the stack that's not a mini-scope
        symtab* save = current_scope;
        while (current_scope->scope_type == SCOPE_MINI)
            st_pop_scope();

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
sym st_define_struct(const char *ident, astn decl_list,
                           YYLTYPE name_context, YYLTYPE closebrace_context, YYLTYPE openbrace_context) {
    sym strunion;
    strunion = st_declare_struct(ident, true, name_context); // strict bc we're about to define!

    //printf("creating mini at %s:%d\n", openbrace_context.filename, openbrace_context.lineno);
    st_new_scope(SCOPE_MINI, openbrace_context);
    strunion->members = current_scope; // mini-scope
    strunion->struct_offset = 0;
    sym member;
    while (decl_list) {
        member = real_begin_st_entry(list_data(decl_list), NS_MEMBERS, list_data(decl_list)->context);
        member->linkage = L_NONE;
        member->storspec = SS_NONE;
        member->struct_offset = strunion->struct_offset++;
        decl_list = list_next(decl_list);
    }
    strunion->def_context = closebrace_context;
    st_pop_scope();
    return strunion;
}


void st_make_union(void) {

}


// ret function scope if in function, else NULL
symtab* st_parent_function(void) {
    symtab* cur = current_scope;
    while (cur->scope_type == SCOPE_BLOCK)
        cur = cur->parent;
    if (cur->scope_type == SCOPE_FUNCTION)
        return cur;
    else
        return NULL;
}


// kludge up the linkage and storage specs for globals
static void st_check_linkage(sym e) {
    if (e->scope == &root_symtab) {
        if (e->storspec == SS_UNDEF) { // plain declaration in global scope
            if (e->type->Type.is_const)// top level type
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


static void check_dtypechain_legality(astn head) {
    // TODO: add context
    // not allowed:
    // - array of function
    // - function returning function
    // - function returning array
    while (head && head->type == ASTN_TYPE && head->Type.is_derived) {
        astn target = head->Type.derived.target;
        switch (head->Type.derived.type) {
            case t_PTR:
                break;
            case t_ARRAY:
            {
                if (target->type == ASTN_TYPE && target->Type.is_derived &&
                    target->Type.derived.type == t_FN) {
                        st_error("Attempted declaration of array of functions\n");
                    }
                break;
            }
            case t_FN:
            {
                if (target->type == ASTN_TYPE && target->Type.is_derived) {
                    if (target->Type.derived.type == t_FN) {
                        st_error("Attempted declaration of function returning function\n");
                    }
                    if (target->Type.derived.type == t_ARRAY) {
                        st_error("Attempted declaration of function returning array\n");
                    }
                }
            }
            default:
                break;
        }
        head = head->Type.derived.target;
    }
}


/*
 *  Synthesize a new st_entry, qualify and specify it, and attempt to install it
 *  into the current scope. Sets entry_type to STE_VAR by default.
 *
 *  TODO: decl is not yet a list
 */
static sym real_begin_st_entry(astn decl, enum namespaces ns, YYLTYPE context) {
    ast_check(decl, ASTN_DECL, "Expected decl to make new st_entry.");

    // Get information from the type chain
    astn type_chain = decl->Decl.type;
    check_dtypechain_legality(type_chain);

    const char *name = get_dtypechain_ident(type_chain);

    // allocate a new entry
    sym new = stentry_alloc(name);

    // set context, namespace, scope, and entry type
    new->decl_context = context;
    new->ns = ns;
    new->scope = current_scope;
    new->entry_type = STE_VAR; // default; override from caller when needed!

    // describe the storage and linkage
    new->storspec = describe_type(decl->Decl.specs, &new->type->Type);

    if (!new->storspec && st_parent_function())
        new->storspec = SS_AUTO;

    st_check_linkage(new);

    // complete the type by flipping the dtypechain
    reset_dtypechain_target(type_chain, new->type); // end of chain is now the type instead of ident
    if (type_chain->type == ASTN_TYPE && type_chain->Type.is_derived) { // ALLOCATE HERE?
        new->type = type_chain; // because otherwise it's just an IDENT
    }

    // attempt to insert the new entry, check for permitted redeclaration
    if (!st_insert_given(new)) {
        if (new->scope == &root_symtab) {
            sym prev = st_lookup_ns(new->ident, new->ns);
            // is previous extern? if so, completely replace it.
            // this logic and behavior is total shit, please fix this some day
            // this whole problem can only be described as disgusting
            // #1 the standard (6.9.2) is nearly incomprehensible on this point
            // #2 it seems like that rule is not always applied evenly across compilers.
            // H&S 4.8.5 was extremely useful on this. I'll try to follow a C++ -like model,
            // in which the only tentative definition is one that's 'extern', and those cannot have initializers.
            if (prev->storspec == SS_EXTERN && new->storspec != SS_EXTERN) {
                sym next = prev->next;
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


sym begin_st_entry(astn decl, enum namespaces ns, YYLTYPE context) {
    if (decl->Decl.type->type == ASTN_TYPE && decl->Decl.type->Type.derived.type == t_FN) {
        sym newfn = st_declare_function(decl, context);
        newfn->fn_defined = false;
        newfn->def_context = (YYLTYPE){NULL, 0}; // it's not defined
        return newfn;
    } else {
        return real_begin_st_entry(decl, ns, context);
    }
}

