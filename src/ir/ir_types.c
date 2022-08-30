#include "ir_types.h"

#include "ir.h"
#include "ir_arithmetic.h"
#include "ir_util.h"

bool is_integer(astn a) {
    if (a->type != ASTN_QTYPE)
        return is_integer(get_qtype(a));

    ir_type_E t = ir_type(a);

    return IR_TYPE_INTEGER_MIN < t && t < IR_TYPE_INTEGER_MAX;
}

/**
 * Returns qtype's ir_type.
 */
ir_type_E ir_type(astn a) {
    if (a->type != ASTN_QTYPE)
        return ir_type(get_qtype(a));

    return a->Qtype.ir_type;
}

/**
 * Returns qtype's derived_type.
 */
astn ir_dtype(astn t) {
    if (t->type != ASTN_QTYPE)
        return ir_dtype(get_qtype(t));

    return get_qtype(t)->Qtype.derived_type;
}

/**
 * Do the ir_types match exactly?
 */
bool ir_type_matches(astn a, ir_type_E t) {
    return ir_type(a) == t;
}

/**
 * Get the ASTN_QTYPE for the given node.
 */
astn get_qtype(astn t) {
    ir_type_E ret;
    astn ret_der = NULL;

    astn n;
    switch (t->type) {
        case ASTN_NUM:
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

        case ASTN_SYMPTR:;
            return get_qtype(t->Symptr.e->type);

        case ASTN_TYPE:
            if (t->Type.is_derived) {
                switch (t->Type.derived.type) {
                    case t_PTR:
                        ret = IR_ptr;
                        ret_der = t->Type.derived.target;
                        break;
                    case t_ARRAY:
                        ret = IR_arr;
                        ret_der = t; // special for arrays - derived_type is the array
                        break;
                    default:
                        die("Invalid derived type in get_qtype");
                }
            } else {
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
            }
            astn q = qtype_alloc(ret);
            q->Qtype.derived_type = ret_der;
            return q;

        case ASTN_QTEMP:
            return t->Qtemp.qtype;

        case ASTN_QTYPE:
            return t;

        case ASTN_UNOP:; // the type of a unop deref is special.
            astn utarget = t->Unop.target;

            if (t->Unop.op != '*')
                qunimpl(t, "Unsupported unop type in get_qtype");

            switch (utarget->type) {
                // things that can yield a pointer
                case ASTN_BINOP:
                case ASTN_SYMPTR:;
                    n = get_qtype(utarget); // should be ptr, PTR TO int

                    if (n->Qtype.derived_type) {
                        ast_check(n->Qtype.derived_type, ASTN_TYPE, "");
                        return get_qtype(n->Qtype.derived_type);
                    } else {
                        qerror("Dereferenced non-pointer symbol!");
                    }

                    break;

                case ASTN_UNOP:;
                    n = get_qtype(utarget); // recurse and get end

                    if (n->Qtype.derived_type->Type.is_derived)
                        n->Qtype.derived_type = get_qtype(n->Qtype.derived_type->Type.derived.target);
                    else // not derived!
                        n = get_qtype(n->Qtype.derived_type);
                    return n;

                default:
                    qunimpl(utarget, "Invalid target of unop deref in get_qtype");
            }

        case ASTN_BINOP:
            qunimpl(t, "Tried to get type of binop!");

        default:
            qunimpl(t, "Unimplemented astn type in get_qtype :(");
    }
}

