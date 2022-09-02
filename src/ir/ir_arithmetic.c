#include "ir_arithmetic.h"

#include "ir.h"
#include "ir_types.h"
#include "ir_util.h"
#include "parser.tab.h"


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
}

astn gen_pointer_addition(astn ptr, astn i, astn target) {
    astn i_ext = convert_integer_type(i, IR_PTR_INT_TYPE);

    target = qprepare_target(target, get_qtype(ptr)); // it's the same type as the pointer

    // now we emit a GEP to perform the indexing
    emit(IR_OP_GEP, target, ptr, i_ext);

    return target;
}

astn gen_pointer_subtraction(astn ptr, astn i, astn target) {
    astn i_ext = convert_integer_type(i, IR_PTR_INT_TYPE);
    astn i_ext_neg = do_negate(i_ext);

    target = qprepare_target(target, get_qtype(ptr));

    emit(IR_OP_GEP, target, ptr, i_ext_neg);

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
        astn conv_l;
        astn conv_r;

        astn res_type = do_arithmetic_conversions(l, r, &conv_l, &conv_r);

        target = qprepare_target(target, res_type);

        emit(IR_OP_ADD, target, conv_l, conv_r);
        return target;

    } else if (l_is_integer && r_is_pointer) {
        return gen_pointer_addition(r, l, target);

    } else if (l_is_pointer && r_is_integer) {
        return gen_pointer_addition(l, r, target);

    } else {
        die("Unimplemented or invalid operands in gen_add_rvalue!");
    }


    return target;
}

astn gen_sub_rvalue(astn a, astn target) {
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
        astn conv_l;
        astn conv_r;

        astn res_type = do_arithmetic_conversions(l, r, &conv_l, &conv_r);

        target = qprepare_target(target, res_type);

        emit(IR_OP_SUB, target, conv_l, conv_r);
        return target;
    }

    if (l_is_pointer && r_is_pointer) {
        qunimpl(a, "Unimplemented: pointer - pointer subtraction.");
    }

    if (l_is_integer && r_is_pointer) {
        qerrorl(a, "Cannot subtract pointer from integer");
    }

    if (l_is_pointer && r_is_integer) {
        return gen_pointer_subtraction(l, r, target);
    }

    qerrorl(a, "Invalid operands to subtraction.");
}

astn do_negate(astn a) {
    if (!is_integer(a))
        qunimpl(a, "Attempted to negate non-integer");

    astn neg = new_qtemp(get_qtype(a)); // it's ok if it's not signed, IR type is same
    emit(IR_OP_SUB, neg, simple_constant_alloc(0), a);
    return neg;
}
