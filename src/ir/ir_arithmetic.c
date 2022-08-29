#include "ir_arithmetic.h"

#include "ir.h"
#include "ir_types.h"
#include "ir_util.h"
#include "parser.tab.h"

bool is_integer(astn a) {
    if (a->type != ASTN_QTYPE)
        return is_integer(get_qtype(a));

    ir_type_E t = a->Qtype.qtype;

    return IR_TYPE_INTEGER_MIN < t && t < IR_TYPE_INTEGER_MAX;
}

ir_type_E ir_type(astn a) {
    if (a->type != ASTN_QTYPE)
        return ir_type(get_qtype(a));

    return a->Qtype.qtype;
}

bool ir_type_matches(astn a, ir_type_E t) {
    return ir_type(a) == t;
}

// 6.2.5.18 - Integer and floating types are collectively called arithmetic types.
// We don't need most of the logic here, use get_qtype instead. The temporary binop implementation is blocking the change.
bool type_is_arithmetic(astn a) {
    switch (a->type) {
        case ASTN_QTEMP:
        case ASTN_QTYPE:
            // for now just integer types
            return is_integer(a);

        case ASTN_SYMPTR:
            return type_is_arithmetic(a->Symptr.e->type);

        case ASTN_TYPE:
            return (!a->Type.is_derived && !a->Type.is_tagtype);

        case ASTN_NUM:
            return true;

        case ASTN_STRLIT:
            die("Unsupported arithmetic type - string literal");
            return true;

        case ASTN_BINOP: // predict arithmeticness of result
                         // you can't just go off of both operands,
                         // because e.g. (ptr < ptr) -> arithmetic
            switch (a->Binop.op) {
                case '*':
                case '/':
                case '%':
                case SHL:
                case SHR:
                case '<':
                case '>':
                case GTEQ:
                case LTEQ:
                case EQEQ:
                case NOTEQ:
                case '&':
                case '^':
                case '|':
                case LOGAND:
                case LOGOR:
                    return true;

                case '+':
                case '-':
                    return (type_is_arithmetic(a->Binop.left) && type_is_arithmetic(a->Binop.right));

                default:
                    qunimpl(a, "Unhandled binop in type_is_arithmetic :(");
            }
            break;

        case ASTN_UNOP:
            switch (a->Unop.op) {
                case '*':; // deref
                    astn d = get_qtype(a)->Qtype.derived_type;
                    if (d)
                        return type_is_arithmetic(d);
                    else
                        return true;
                default:
                    qunimpl(a, "Unhandled unop in type_is_arithmetic :(");
            }

        default:
            qunimpl(a, "Unsupported astn for type_is_arithmetic :(");
    }

    die("Unreachable");
}

astn gen_pointer_addition(astn ptr, astn i, astn target) {
    target = qprepare_target(target, get_qtype(ptr)); // it's the same type as the pointer

    // now we emit a GEP to perform the indexing
    emit(IR_OP_GEP, target, ptr, i);

    return target;
}

astn gen_add_rvalue(astn a, astn target) {
    // 6.5.6 Additive operators
    // constraints - addition:
    // - either both operands will have arithmetic type,
    //   or one operand shall be a pointer to an object
    //   type and the other shall have integer type.
    astn l = gen_rvalue(a->Binop.left, NULL);
    astn r = gen_rvalue(a->Binop.right, NULL);

    astn l_type = get_qtype(l);
    astn r_type = get_qtype(r);

    bool l_is_arith = type_is_arithmetic(l);
    bool r_is_arith = type_is_arithmetic(r);

    bool l_is_integer = is_integer(l_type);
    bool r_is_integer = is_integer(r_type);

    bool l_is_pointer = ir_type_matches(l_type, IR_ptr);
    bool r_is_pointer = ir_type_matches(r_type, IR_ptr);

    if (l_is_arith && r_is_arith) {
        // check here for type of operation
        if (ir_type(l_type) != ir_type(r_type))
        {
            // lift types here
            die("Unimplemented: type lifting :(");
        }

        target = qprepare_target(target, l_type); // resultant type

        emit(IR_OP_ADD, target, l, r);
        return target;

    } else if (l_is_integer && r_is_pointer) {
        qwarn("Got integer + ptr");
        return gen_pointer_addition(r, l, target);

    } else if (l_is_pointer && r_is_integer) {
        qwarn("Got ptr + integer");
        return gen_pointer_addition(l, r, target);

    } else {
        die("Unimplemented or invalid operands in gen_add_rvalue!");
    }


    return target;
}

astn gen_sub_rvalue(astn a, astn target) {
    (void)target;
    qunimpl(a, "Unimplemented: gen_sub_rvalue :(");
}


