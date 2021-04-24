#include "symtab.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util.h"

#define st_error(...) \
    fprintf(stderr, "Error declaring symbol: " __VA_ARGS__); \
    exit(-5);

// symtab for this translation unit
symtab root_symtab = {
    .scope_type = SCOPE_FILE,
    .parent     = NULL,
    .first      = NULL,
    .last       = NULL
};

symtab *current_scope = &root_symtab;

// in theory, you can destroy the spec chain after this function
// but who does memory management anyway
// decl_list is not yet a list!
void begin_st_entry(astn *spec, astn *decl_list) {
    // the below will need revising for lists, and also some error checking
    st_entry *new = stentry_alloc(get_dtypechain_target(decl_list)->astn_ident.ident);

    struct astn_type *t = &new->type->astn_type;
    new->storspec = specify_type(spec, t);
    reset_dtypechain_target(decl_list, new->type); // end of chain is now the type instead of ident

    if (decl_list->type == ASTN_TYPE && decl_list->astn_type.is_derived) {
        new->type = decl_list; // because otherwise it's just an IDENT
    }
    //printf("Installing symbol <%s> into symbol table, storage class <%s>, with below type:\n", \
            new->ident, storage_specs_str[new->type->astn_type.scalar.storspec]);
    print_ast(new->type);

    if (!st_insert_given(new)) {
        st_error("attempted redeclaration of symbol %s\n", new->ident);
    }
}

// apply type specifiers to a type, return storage specifier
// also applies qualifiers!
enum storspec specify_type(astn *spec, struct astn_type *t) {
    unsigned VOIDs=0, CHARs=0, SHORTs=0, INTs=0, LONGs=0, FLOATs=0;
    unsigned DOUBLEs=0, SIGNEDs=0, UNSIGNEDs=0, BOOLs=0, COMPLEXs=0;
    unsigned total_typespecs = 0;
    enum storspec storspec = SS_AUTO;
    bool storspec_set = false;
    while (spec) { // we'll do more validation later of the typespecs later
        if (spec->type == ASTN_TYPESPEC) {
            switch (spec->astn_typespec.spec) {
                case TS_VOID:       VOIDs++;        break;
                case TS_CHAR:       CHARs++;        break;
                case TS_SHORT:      SHORTs++;       break;
                case TS_INT:        INTs++;         break;
                case TS_LONG:       LONGs++;        break;
                case TS_FLOAT:      FLOATs++;       break;
                case TS_DOUBLE:     DOUBLEs++;      break;
                case TS_SIGNED:     SIGNEDs++;      break;
                case TS_UNSIGNED:   UNSIGNEDs++;    break;
                case TS__BOOL:      BOOLs++;        break;
                case TS__COMPLEX:   COMPLEXs++;     break;
                default:    die("invalid typespec");
            }
            total_typespecs++;
            astn* old = spec;
            spec = spec->astn_typespec.next;
            free(old); // maybe a little memory management
        } else if (spec->type == ASTN_TYPEQUAL) { // specifying multiple times is valid
            switch (spec->astn_typequal.qual) {
                case TQ_CONST:      t->is_const = true;       break;
                case TQ_RESTRICT:   t->is_restrict = true;    break;
                case TQ_VOLATILE:   t->is_volatile = true;    break;
                default:    die("invalid typequal");
            }
            astn* old = spec;
            spec = spec->astn_typequal.next;
            free(old);
        } else if (spec->type == ASTN_STORSPEC) {
            if (storspec_set) { // better error handling later
                st_error("cannot specify more storage class more than once\n");
            }
            storspec = spec->astn_storspec.spec;
            //printf("dbg - storspec astn has %d, so %s", spec->astn_storspec.spec, storage_specs_str[t->scalar.storspec]);
            storspec_set = true;
            astn* old = spec;
            spec = spec->astn_storspec.next;
            free(old);
        } else {
            die("Invalid astn type in spec chain");
        }
    }

    // validate and parse type specifiers into a single type + signedness
    if (UNSIGNEDs > 1 || SIGNEDs > 1) {
        st_error("cannot specify 'signed' or 'unsigned' more than once\n");
    }
    if (UNSIGNEDs && SIGNEDs) {
        st_error("cannot combine 'signed' and 'unsigned'\n");
    }
    if ((VOIDs + CHARs + INTs + FLOATs + DOUBLEs + BOOLs) > 1) { // long and short not included
        st_error("cannot specify base type more than once\n");
    }

    if (UNSIGNEDs) {
        t->scalar.is_unsigned = true;
        t->scalar.type = t_INT; // may change if more specifiers
        total_typespecs--;
    }
    if (SIGNEDs) {
        t->scalar.is_unsigned = false; // this is redundant
        t->scalar.type = t_INT; // may change if more specifiers
        total_typespecs--;
    }

    if (VOIDs) {
        total_typespecs--;
        if (total_typespecs) {
            st_error("cannot combine 'void' with other type specifiers\n");
        }
        t->scalar.type = t_VOID;
    } else if (CHARs) {
        total_typespecs--;
        if (total_typespecs) {
            st_error("invalid combination of type specifiers\n");
        }
        t->scalar.type = t_CHAR;
    } else if (SHORTs) {
        if (SHORTs == 1) {
            total_typespecs--;
            if (INTs) total_typespecs--; // we checked that there's only one earlier
        } else {
            st_error("cannot specify 'short' more than once\n");
        }
        if (total_typespecs) { // maybe we should die here instead, because I don't see how this would happen naturally
            st_error("invalid combination of type specifiers\n");
        }
        t->scalar.type = t_SHORT;
    } else if (LONGs) {
        if (INTs) total_typespecs--;
        if (LONGs == 1) {
            total_typespecs--;
            if (DOUBLEs) {
                total_typespecs--;
                if (COMPLEXs) {
                    total_typespecs--;
                    t->scalar.type = t_LONGDOUBLECPLX;
                } else {
                    t->scalar.type = t_LONGDOUBLE;
                }
                goto long_end;
            }
            t->scalar.type = t_LONG;
        } else if (LONGs == 2) {
            total_typespecs -= 2;
            t->scalar.type = t_LONGLONG;
        } else {
            st_error("'long' cannot be specified more than twice\n");
        }
long_end:
        if (total_typespecs) {
            st_error("invalid combination of type specifiers\n");
        }
    } else if (FLOATs) {
        total_typespecs--;
        if (COMPLEXs) {
            total_typespecs--;
            t->scalar.type = t_FLOATCPLX;
        } else {
            t->scalar.type = t_FLOAT;
        }
        if (total_typespecs) {
            st_error("invalid combination of type specifiers\n");
        }
    } else if (DOUBLEs) {
        total_typespecs--;
        if (total_typespecs) {
            st_error("invalid combination of type specifiers\n");
        }
        t->scalar.type = t_DOUBLE;
    } else if (BOOLs) {
        total_typespecs--;
        if (total_typespecs) {
            st_error("invalid combination of type specifiers\n");
        }
        t->scalar.type = t_BOOL;
    } else if (INTs) {
        total_typespecs--;
        if (total_typespecs) {
            st_error("invalid combination of type specifiers\n");
        }
        t->scalar.type = t_INT;
    } else {
        st_error("invalid combination of type specifiers\n");
    }
    return storspec;
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
