#include "ir.h"
#include "ir_arithmetic.h"
#include "ir_loadstore.h"
#include "ir_state.h"
#include "ir_types.h"
#include "ir_util.h"

#include <string.h>

#include "ast.h"
#include "ast_print.h"
#include "parser.tab.h"
#include "symtab.h"
#include "types.h"
#include "util.h"

static struct BB root_bb = {0};
static struct BBL root_bbl = {.me = &root_bb};

struct ir_state irst = {
    .root_bbl = &root_bbl,
    .bb = &root_bb,
};


astn gen_indirection(astn a) {
    ast_check(a, ASTN_UNOP, "");
    if (a->Unop.op != '*')
        die("Passed wrong unop type to gen_indirection");

    astn targ_rval = gen_rvalue(a->Unop.target, NULL);

    if (!ir_type_matches(targ_rval, IR_ptr))
        qerror("Object to be dereferenced is not a pointer.");

    // if the operand points to a function, the result is a function designator.
    // TODO

    // if it points to an object, the result is an lvalue designating the object.
    return gen_lvalue(targ_rval);
}

astn lvalue_to_rvalue(astn a, astn target) {
    switch (a->type) {
        case ASTN_QTEMP:;
            astn obj_type = ir_dtype(a); // e.g. a is alloca, obj_type is the array
            obj_type = get_qtype(obj_type);

            if (ir_type_matches(obj_type, IR_arr)) {

                // decay the underlying type
                // ptr -> array[...]
                // to
                // ptr -> ...
                astn arr = ir_dtype(obj_type);
                ast_check(arr, ASTN_TYPE, "");

                astn arr_targ = arr->Type.derived.target;

                astn ptr_type = qtype_alloc(IR_ptr);
                ptr_type->Qtype.derived_type = arr_targ;

                if (target)
                    die("why target non-null");

                target = qprepare_target(target, ptr_type);
                emit4(IR_OP_GEP, target, a, simple_constant_alloc(0), simple_constant_alloc(0));
                return target;
            }

            return gen_load(a, target);

        default:
            qunimpl(a, "Unsupported astn type for lvalue_to_rvalue!");
    }
}

static astn _gen_rvalue(astn a, astn target) {
    switch (a->type) {
        case ASTN_NUM:
            return a;

        case ASTN_BINOP:
            switch (a->Binop.op) {
                case '+':
                    return gen_add_rvalue(a, target);
                case '-':
                    return gen_sub_rvalue(a, target);
                default:
                    qunimpl(a, "Unhandled binop type for gen_rvalue :(");
            }

        case ASTN_UNOP:
            switch (a->Unop.op) {
                case '*':
                    return lvalue_to_rvalue(gen_indirection(a), target);

                default:
                    qunimpl(a, "Unhandled unop in gen_rvalue :(");
            }

        case ASTN_SYMPTR:
           return lvalue_to_rvalue(gen_lvalue(a), target);

        case ASTN_QTEMP:
            return a;

        default:
            qunimpl(a, "Unhandled astn for gen_rvalue :(");
    }

    qunimpl(a, "Unimplemented astn in gen_rvalue :(");
}

astn gen_rvalue(astn a, astn target) {
    astn r = _gen_rvalue(a, target);
    return r;
}

void gen_quads(astn a) {
    switch (a->type) {
        case ASTN_RETURN:
            emit(IR_OP_RETURN, NULL, gen_rvalue(a->Return.ret, NULL), NULL);
            break;

        case ASTN_DECLREC:
            // generate initializers
            // assuming local scope
            if (a->Declrec.init) {
                astn ass = astn_alloc(ASTN_ASSIGN);
                ass->Assign.left = symptr_alloc(a->Declrec.e);
                ass->Assign.right = a->Declrec.init;
                gen_assign(ass);
            }
            break;

        case ASTN_BINOP:
        case ASTN_UNOP:
            qwarn("Warning: useless expression: ");
            print_ast(a);
            gen_rvalue(a, NULL);
            break;

        case ASTN_FNCALL:
            gen_rvalue(a, NULL);
            break;

        case ASTN_NOOP:
            break;

        case ASTN_ASSIGN:
            gen_assign(a);
            break;

        default:
            print_ast(a);
            die("Unimplemented astn for quad generation");
    }
}

void gen_global(sym e) {
    astn qtype = qtype_alloc(IR_ptr);

    qtype->Qtype.derived_type = e->type;

    astn qtemp = qtemp_alloc(-1, qtype);

    // double-link them to each other
    qtemp->Qtemp.global = symptr_alloc(e);
    e->ptr_qtemp = qtemp;

    emit(IR_OP_DEFGLOBAL, qtemp, NULL, NULL);
}

void gen_fn(sym e) {
    irst.fn = e;
    irst.bb = bb_alloc();

    // generate local allocations
    sym n = e->fn_scope->first;
    while (n) {
        if (n->entry_type == STE_VAR && n->storspec == SS_AUTO) {
            astn qtemp = new_qtemp(qtype_alloc(IR_ptr));
            qtemp->Qtemp.qtype->Qtype.derived_type = get_qtype(symptr_alloc(n));
            n->ptr_qtemp = qtemp;

            emit(IR_OP_ALLOCA, qtemp, NULL, NULL);
        }

        n = n->next;
    }

    astn a = e->body;
    ast_check(a, ASTN_LIST, "");

    // generate body quads
    while (a && list_data(a)) {
        gen_quads(list_data(a));
        a = list_next(a);
    }

    // check return - should check non-main too
    const_quad const last = last_in_bb(irst.bb);
    if (!last || last->op != IR_OP_RETURN) {
        if (!strcmp(irst.fn->ident, "main")) {
            emit(IR_OP_RETURN, NULL, gen_rvalue(simple_constant_alloc(0), NULL), NULL);
        }
    }
}

