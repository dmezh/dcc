#include "ir.h"
#include "ir_print.h"
#include "ir_util.h"

#include <string.h>

#include "ast.h"
#include "ast_print.h"
#include "symtab.h"
#include "types.h"
#include "util.h"

struct BB bb_root = { .bbno = -1 };
BB current_bb = &bb_root;

ir_type_E type_to_ir(const_astn t) {
    if (t->type == ASTN_SYMPTR)
        return type_to_ir(t->Symptr.e->type);

    ast_check(t, ASTN_TYPE, "");

    if (t->Type.is_derived && t->Type.derived.type == t_PTR)
        return IR_ptr;

    switch (t->Type.scalar.type) {
        case t_INT: return IR_i32;
        case t_LONG: return IR_i64;
        case t_CHAR: return IR_i8;
        case t_SHORT: return IR_i16;
        default:
            fprintf(stderr, "UH OH:\n");
            print_ast(t);
            die("Unsupported type in IR :(");
    }

    die("Unreachable");
    return IR_TYPE_UNDEF;
}

quad last_in_bb(BB b) {
    quad q = b->current;
    if (!q) return NULL;

    while (q->next) q = q->next;

    return q;
}

astn gen_rvalue(astn a, astn target) {
    (void)target;

    switch (a->type) {
        case ASTN_NUM:
            return a;

        default:
            die("Unhandled astn for gen_rvalue :(");
    }

    return NULL;
}

void gen_quads(astn a) {
    switch (a->type) {
        case ASTN_RETURN:
            emit(IR_OP_RETURN, NULL, gen_rvalue(a->Return.ret, NULL), NULL);
            break;

        case ASTN_DECLREC:
            break;

        default:
            print_ast(a);
            die("Unimplemented astn for quad generation");
    }
}

void gen_fn(sym e) {
    current_bb->fn = e;

    // generate local allocations
    sym n = e->fn_scope->first;
    int qtemp_no = 1;
    while (n) {
        if (n->entry_type == STE_VAR && n->storspec == SS_AUTO) {
            fprintf(stderr, ";   found local variable %s with size %d, IR type %s\n",
                            n->ident,
                            get_sizeof(symptr_alloc(n)),
                            ir_type_str[type_to_ir(n->type)]);

            astn qtemp = qtemp_alloc(qtemp_no++);
            astn irtypestr = qtype_alloc(type_to_ir(n->type));

            emit(IR_OP_ALLOCA, qtemp, irtypestr, NULL);
        }

        n = n->next;
    }

    astn a = e->body;
    ast_check(a, ASTN_LIST, "");

    while (a && list_data(a)) {
        gen_quads(list_data(a));
        a = list_next(a);
    }
}

