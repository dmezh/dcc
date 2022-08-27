#include "ir.h"
#include "ir_print.h"
#include "ir_state.h"
#include "ir_util.h"

#include <string.h>

#include "ast.h"
#include "ast_print.h"
#include "symtab.h"
#include "types.h"
#include "util.h"

struct ir_state irst = {
    .root_bbl = &(struct BBL){0},
    .bb = &(struct BB){0},
};

astn get_qtype(const_astn t) {
    ir_type_E ret;

    if (t->type == ASTN_NUM) {
        switch (t->Num.number.aux_type) {
            case s_INT:
                ret = IR_i32;
                break;
            case s_LONG:
                ret = IR_i64;
                break;
            case s_CHARLIT:
                ret = IR_i8;
                break;
            default:
                ret = IR_TYPE_UNDEF;
                qunimpl(t, "Unsupported number literal int_type in IR:(");
        }

        return qtype_alloc(ret);
    }

    if (t->type == ASTN_SYMPTR)
        return get_qtype(t->Symptr.e->type);

    ast_check(t, ASTN_TYPE, "");

    if (t->Type.is_derived && t->Type.derived.type == t_PTR)
        ret = IR_ptr;

    switch (t->Type.scalar.type) {
        case t_INT:
            ret = IR_i32;
            break;
        case t_LONG:
            ret = IR_i64;
            break;
        case t_CHAR:
            ret = IR_i8;
            break;
        case t_SHORT:
            ret = IR_i16;
            break;
        default:
            qunimpl(t, "Unsupported type in IR :(");
    }

    return qtype_alloc(ret);
}

// 6.2.5.18 - Integer and floating types are collectively called arithmetic types.
bool type_is_arithmetic(astn a) {
    switch (a->type) {
        case ASTN_SYMPTR:
            return type_is_arithmetic(a->Symptr.e->type);

       case ASTN_TYPE:
            return (!a->Type.is_derived && !a->Type.is_tagtype);

        case ASTN_NUM:
            return true;

        case ASTN_STRLIT:
            die("Unsupported arithmetic type - string literal");
            return true;

        default:
            qunimpl(a, "Unsupported astn for type_is_arithmetic :(");
    }

    die("Unreachable");
    return false;
}

astn gen_add_rvalue(astn a, astn target) {
    // 6.5.6 Additive operators
    // constraints - addition:
    // - either both operands will have arithmetic type,
    //   or one operand shall be a pointer to an object
    //   type and the other shall have integer type.
    astn l = a->Binop.left;
    astn r = a->Binop.right;

    if (type_is_arithmetic(l) && type_is_arithmetic(r)) {
        // check here for type of operation
        if (get_qtype(l)->Qtype.qtype != get_qtype(r)->Qtype.qtype)
        {
            // lift types here
            die("Unimplemented: type lifting :(");
        }

        if (!target)
            target = new_qtemp(get_qtype(l));

        emit(IR_OP_ADD, target, l, r);
    }


    return target;
}

astn gen_sub_rvalue(astn a, astn target) {
    (void)target;
    qwarn("UH OH:\n:");
    print_ast(a);
    die("Unimplemented: gen_sub_rvalue :(");
    return NULL;
}

astn gen_rvalue(astn a, astn target) {
    (void)target;

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
                    qwarn("UH OH:\n");
                    print_ast(a);
                    die("Unhandled binop type for gen_rvalue :(");
            }
            break;

        default:
            qwarn("UH OH:\n");
            print_ast(a);
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

        case ASTN_BINOP:
        case ASTN_UNOP:
            qwarn("Warning: useless expression: ");
            print_ast(a);
            gen_rvalue(a, NULL);
            break;

        case ASTN_FNCALL:
            gen_rvalue(a, NULL);
            break;

        default:
            print_ast(a);
            die("Unimplemented astn for quad generation");
    }
}

void gen_fn(sym e) {
    irst.fn = e;
    irst.bb = bb_alloc();

    // generate local allocations
    sym n = e->fn_scope->first;
    while (n) {
        if (n->entry_type == STE_VAR && n->storspec == SS_AUTO) {
            astn qtemp = new_qtemp(get_qtype(symptr_alloc(n)));

            emit(IR_OP_ALLOCA, qtemp, NULL, NULL);
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

