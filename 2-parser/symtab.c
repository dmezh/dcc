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

symtab *current_scope = &root_symtab;

// in theory, you can destroy the spec chain after this function
// but who does memory management anyway
void begin_st_entry(astn *spec, astn *decl_list) {
    // the below will need revising for lists, and also some error checking
    st_entry *new = stentry_alloc(get_ptrchain_target(decl_list)->astn_ident.ident);
    struct astn_type *t = &new->type->astn_type;
    // first, we will get signed/unsigned
    astn *next = spec;
    unsigned VOIDs=0, CHARs=0, SHORTs=0, INTs=0, LONGs=0, FLOATs=0;
    unsigned DOUBLEs=0, SIGNEDs=0, UNSIGNEDs=0, BOOLs=0, COMPLEXs=0;
    unsigned total_typespecs = 0;
    bool storspec_set = false;
    while (next) { // we'll do more validation later of the typespecs later
        if (next->type == ASTN_TYPESPEC) {
            switch (next->astn_typespec.spec) {
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
            astn* old = next;
            next = next->astn_typespec.next;
            free(old); // maybe a little memory management
        } else if (next->type == ASTN_TYPEQUAL) { // specifying multiple times is valid
            switch (next->astn_typequal.qual) {
                case TQ_CONST:      t->is_const = true;       break;
                case TQ_RESTRICT:   t->is_restrict = true;    break;
                case TQ_VOLATILE:   t->is_volatile = true;    break;
            }
            astn* old = next;
            next = next->astn_typequal.next;
            free(old);
        } else if (next->type == ASTN_STORSPEC) {
            if (storspec_set) { // better error handling later
                fprintf(stderr, "Error: cannot specify more storage class more than once\n");
                exit(-5);
            }
            t->scalar.storspec = next->astn_storspec.spec;
            storspec_set = true;
            astn* old = next;
            next = next->astn_storspec.next;
            free(old);
        } else {
            die("Invalid astn type in spec chain");
        }
    }

    // validate and parse type specifiers into a single type + signedness
    if (UNSIGNEDs > 1 || SIGNEDs > 1) {
        fprintf(stderr, "Error: cannot specify 'signed' or 'unsigned' more than once\n");
        exit(-5);
    }
    if (UNSIGNEDs && SIGNEDs) {
        fprintf(stderr, "Error: cannot combine 'signed' and 'unsigned'\n");
        exit(-5);
    }
    if ((VOIDs + CHARs + INTs + FLOATs + DOUBLEs + BOOLs) > 1) { // long and short not included
        fprintf(stderr, "Error: cannot specify base type more than once\n");
        exit(-5);
    }

    if (UNSIGNEDs) {
        t->scalar.is_unsigned = true;
        t->scalar.type = t_INT; // may change
        total_typespecs--;
    }
    if (SIGNEDs) {
        t->scalar.is_unsigned = false; // this is redundant
        t->scalar.type = t_INT; // may change
        total_typespecs--;
    }

    if (VOIDs) {
        total_typespecs--;
        if (total_typespecs) {
            fprintf(stderr, "Error: cannot combine 'void' with other type specifiers\n");
            exit(-5);
        }
        t->scalar.type = t_VOID;
    } else if (CHARs) {
        total_typespecs--;
        if (total_typespecs) {
            fprintf(stderr, "Error: invalid combination of type specifiers\n");
            exit(-5);
        }
        t->scalar.type = t_CHAR;
    } else if (SHORTs) {
        if (SHORTs == 1) {
            total_typespecs--;
            if (INTs) total_typespecs--; // we checked that there's only one earlier
        } else {
            fprintf(stderr, "Error: cannot specify 'short' more than once\n");
            exit(-5);
        }
        if (total_typespecs) { // maybe we should die here instead, because I don't see how this would happen naturally
            fprintf(stderr, "Error: invalid combination of type specifiers\n");
            exit(-5);
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
            fprintf(stderr, "Error: 'long' cannot be specified more than twice\n");
        }
long_end:
        if (total_typespecs) {
            fprintf(stderr, "Error: invalid combination of type specifiers\n");
            exit(-5);
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
            fprintf(stderr, "Error: invalid combination of type specifiers\n");
            exit(-5);
        }
    } else if (DOUBLEs) {
        total_typespecs--;
        if (total_typespecs) {
            fprintf(stderr, "Error: invalid combination of type specifiers\n");
            exit(-5);
        }
        t->scalar.type = t_DOUBLE;
    } else if (BOOLs) {
        total_typespecs--;
        if (total_typespecs) {
            fprintf(stderr, "Error: invalid combination of type specifiers\n");
            exit(-5);
        }
        t->scalar.type = t_BOOL;
    } else if (INTs) {
        total_typespecs--;
        if (total_typespecs) {
            fprintf(stderr, "Error: invalid combination of type specifiers\n");
            exit(-5);
        }
        t->scalar.type = t_INT;
    } else {
        fprintf(stderr, "Error: invalid combination of type specifiers\n");
        exit(-5);
    }
    //printf("FINALLY, the deduced type is: %d with signedness %d\n", t->scalar.type, !t->scalar.is_unsigned);
    // enter into symtab one day
    reset_ptrchain_target(decl_list, new->type);
    if (decl_list->type == ASTN_TYPE && decl_list->astn_type.is_derived) {
        //decl_list = decl_list->astn_type.derived.target;
        new->type = decl_list; // because otherwise it's just an IDENT
    }
    printf("Installing symbol %s into symbol table with below type:\n", new->ident);
    print_ast(new->type);
    if (!st_insert_given(new)) {
        fprintf(stderr, "Error: attempted redeclaration of symbol %s\n", new->ident);
        exit(-5);
    }
    //printf("ABOUT TO RET, PRINTING AST AT HEAD\n");
    //print_ast(decl_list);
    /* printf("I counted %d VOIDs, %d CHARs, %d SHORTs, %d INTs, %d LONGs, %d FLOATs,\n" \
            "%d DOUBLEs, %d SIGNEDs, %d UNSIGNEDs, %d BOOLs, %d COMPLEXs\n", \
            VOIDs, CHARs, SHORTs, INTs, LONGs, FLOATs, DOUBLEs, SIGNEDs, UNSIGNEDs, BOOLs, COMPLEXs);
    */
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
