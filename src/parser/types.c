/*
 * types.c
 *
 * Utilities for the qualification and specification of types.
 */

#include "types.h"
#include "target_types.h"

#include <stdlib.h>

#include "ast.h"
#include "symtab.h"
#include "symtab_util.h"
#include "util.h"

const char* storspec_str[] = {
    [SS_UNDEF] = "UNDEF!",
    [SS_NONE] = "NONE",
    [SS_AUTO] = "auto",
    [SS_EXTERN] = "extern",
    [SS_STATIC] = "static",
    [SS_REGISTER] = "register",
    [SS_TYPEDEF] = "typedef"
};

const char* der_types_str[] = {
    [t_PTR] = "PTR",
    [t_ARRAY] = "ARRAY",
    [t_FN] = "FN"
};

// sizeof!
// currently, array sizes are hardcoded to only support regular numbers,
// we need a bit more machinery, specifically a function to evaluate compile-time constants.
int get_sizeof(astn type) {
    //printf("getting size of:\n");
    //print_ast(type);
    if (type->type == ASTN_SYMPTR) return (get_sizeof(type->Symptr.e->type));

    ast_check(type, ASTN_TYPE, "sizeof was given a non-type astn.");

    if (!type->Type.is_derived)
        return target_size[type->Type.scalar.type];
    else {
        if (type->Type.derived.type == t_PTR) {
            return target_size[t_PTR];
        }
        // add t_FN stuff
        else {
            if (!type->Type.derived.size) {
                return 0;
            }
            if (type->Type.derived.size->type != ASTN_NUM) {
                eprintf("Sorry, can't yet evaluate compile-time constants other than plain numbers\n");
            }
            return type->Type.derived.size->Num.number.integer * get_sizeof(type->Type.derived.target);
        }
    }
}

// get the first target of an array
// please only call this from the grammar for array_subscript, as it makes assumptions!
astn descend_array(astn type) {
    //printf("dumping TYPE in start\n"); print_ast(type);
    if (type->type == ASTN_SYMPTR) return descend_array(type->Symptr.e->type);
    if (type->type == ASTN_UNOP && type->Unop.op == '*') {
        type = type->Unop.target->Binop.left->Symptr.e->type->Type.derived.target;
    }
    if (type->type == ASTN_TYPE) {
        if (!type->Type.is_derived || type->Type.derived.type != t_ARRAY)
            die("Passed a non-array to array-specific function");
        return type->Type.derived.target;
    }
    die("Passed invalid astn to descend_array");
    return NULL;
}

static astn qualify_single(astn qual, struct astn_type *t);

// apply type specifiers AND qualifiers to a type, return storage specifier
enum storspec describe_type(astn spec, struct astn_type *t) {
    unsigned VOIDs=0, CHARs=0, SHORTs=0, INTs=0, LONGs=0, FLOATs=0;
    unsigned DOUBLEs=0, SIGNEDs=0, UNSIGNEDs=0, BOOLs=0, COMPLEXs=0;
    unsigned tagtypes=0;
    unsigned total_typespecs = 0;

    //print_ast(spec);
    enum storspec storspec = SS_UNDEF;
    bool storspec_set = false;

    while (spec) { // we'll do more validation later of the typespecs later
        if (spec->type == ASTN_TYPESPEC) {
            if (spec->Typespec.is_tagtype) {
                t->is_tagtype = true;
                t->tagtype.symbol = spec->Typespec.symbol;
                astn old = spec;
                spec = spec->Typespec.next;
                free(old);
                tagtypes++;
            } else {
                switch (spec->Typespec.spec) {
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
                astn old = spec;
                spec = spec->Typespec.next;
                free(old); // maybe a little memory management
            }
        } else if (spec->type == ASTN_TYPEQUAL) { // specifying multiple times is valid
            spec = qualify_single(spec, t);
        } else if (spec->type == ASTN_STORSPEC) {
            if (storspec_set) { // better error handling later
                st_error("cannot specify more storage class more than once\n");
            }
            storspec = spec->Storspec.spec;
            //printf("dbg - storspec astn has %d, so %s", spec->Storspec.spec, storspec_str[t->scalar.storspec]);
            storspec_set = true;
            astn old = spec;
            spec = spec->Storspec.next;
            free(old);
        } else {
            die("Invalid astn type in spec chain");
        }
    }

    // I am unsure if this is ugly and bad or not, but it works

    // validate and parse type specifiers into a single type + signedness
    if (tagtypes) {
        if (tagtypes > 1) {
            st_error("cannot specify struct/union/enum more than once\n");
        } else {
            if (total_typespecs) {
                st_error("cannot combine struct/union/enum with other type specifiers\n");
            }
        }
        return storspec;
    }

    if (UNSIGNEDs > 1 || SIGNEDs > 1) {
        st_error("cannot specify 'signed' or 'unsigned' more than once\n");
    }
    if (UNSIGNEDs && SIGNEDs) {
        st_error("cannot combine 'signed' and 'unsigned'\n");
    }
    if ((VOIDs + CHARs + INTs + FLOATs + DOUBLEs + BOOLs) > 1) { // long and short not included
        st_error("cannot specify base type more than once\n");
    }

    if (t->is_const) {
        t->scalar.type = t_INT; // may change if more specifiers
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
    }
    return storspec;
}

// we expect a chain of ONLY qualifiers - this is used in the grammar.
// a non-qual astn here is an INTERNAL error!!
void strict_qualify_type(astn qual, struct astn_type *t) {
    while (qual) {
        if (qual->type == ASTN_TYPEQUAL) {
            qual = qualify_single(qual, t);
        } else {
            die("non-qual astn in allegedly qual chain");
        }
    }
}

// qualify a type from a single astn qual and return the next node
// destroys qual!
static astn qualify_single(astn qual, struct astn_type *t) {
    switch (qual->Typequal.qual) {
        case TQ_CONST:      t->is_const = true;       break;
        case TQ_RESTRICT:   t->is_restrict = true;    break;
        case TQ_VOLATILE:   t->is_volatile = true;    break;
        default:    die("invalid typequal");
    }
    astn old = qual;
    qual = qual->Typequal.next;
    free(old);
    return qual;
}
