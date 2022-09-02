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
    bool is_signed;

    switch (t->type) {
        case ASTN_NUM:;
            is_signed = t->Num.number.is_signed;
            switch (t->Num.number.aux_type) {
                case s_INT:
                    ret = is_signed ? IR_i32 : IR_u32;
                    break;
                case s_LONG:
                    ret = is_signed ? IR_i64 : IR_u64;
                    break;
                case s_LONGLONG:
                    ret = is_signed ? IR_i64 : IR_u64;
                    break;
                case s_CHARLIT:
                    ret = is_signed ? IR_i8 : IR_u8; // is this ever signed?
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
                        qunimpl(t, "Invalid derived type in get_qtype");
                }
            } else {
                is_signed = !t->Type.scalar.is_unsigned;
                switch (t->Type.scalar.type) {
                    case t_INT:
                        ret = is_signed ? IR_i32 : IR_u32;
                        break;
                    case t_LONG:
                        ret = is_signed ? IR_i64 : IR_u64;
                        break;
                    case t_LONGLONG:
                        ret = is_signed ? IR_i64 : IR_u64;
                        break;
                    case t_CHAR:
                        ret = is_signed ? IR_i8 : IR_u8;
                        break;
                    case t_SHORT:
                        ret = is_signed ? IR_i16 : IR_u16;
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

                    if (ir_dtype(n)) {
                        ast_check(ir_dtype(n), ASTN_TYPE, "");
                        return get_qtype(ir_dtype(n));
                    } else {
                        qerrorl(n, "Dereferenced non-pointer symbol!");
                    }

                    break;

                case ASTN_UNOP:;
                    n = get_qtype(utarget); // recurse and get end

                    if (ir_dtype(n)->Type.is_derived)
                        n->Qtype.derived_type = get_qtype(ir_dtype(n)->Type.derived.target);
                    else // not derived!
                        n = get_qtype(ir_dtype(n));
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

static bool type_is_signed[IR_TYPE_INTEGER_MAX] = {
    [IR_u8] = false,
    [IR_i8] = true,
    [IR_u16] = false,
    [IR_i16] = true,
    [IR_u32] = false,
    [IR_i32] = true,
    [IR_u64] = false,
    [IR_i64] = true,
};

astn make_type_compat_with(astn a, astn kind) {
    bool kind_is_ptr = ir_type_matches(kind, IR_ptr);
    bool a_is_ptr = ir_type_matches(a, IR_ptr);

    bool kind_is_integer = is_integer(kind);
    bool a_is_integer = is_integer(a);

    if (kind_is_ptr) {
        return convert_to_ptr(a, kind);
    }

    if (kind_is_integer && a_is_integer) {
        return convert_integer_type(a, ir_type(kind));
    }

    if (kind_is_integer && a_is_ptr) {
        return ptr_to_int(a, kind);
    }

    qunimpl(kind, "Don't know how to make something compatible with type.");
}

astn ptr_to_int(astn a, astn kind) {
    if (!ir_type_matches(a, IR_ptr))
        qunimpl(a, "Passed non-ptr to ptr_to_int!");

    if (!is_integer(kind))
        qunimpl(kind, "Passed non-integer kind to ptr_to_int!");

    astn irt = qtype_alloc(ir_type(kind));
    astn target = qprepare_target(NULL, irt);

    emit(IR_OP_PTRTOINT, target, a, NULL);

    return target;
}

astn convert_to_ptr(astn a, astn kind) {
    if (ir_type_matches(a, IR_ptr)) {
        // equalize derived types.

        ast_check(a, ASTN_QTEMP, "Non-qtemp parameter a ptr passed to convert_to_ptr");

        astn q = qtemp_alloc(a->Qtemp.tempno, get_qtype(kind)); // tempno of a, qtype of kind

        return q;
    }

    if (is_integer(a)) {
        astn rval = gen_rvalue(a, NULL);

        astn ptr = qprepare_target(NULL, qtype_alloc(IR_ptr));
        ptr->Qtype.derived_type = ir_dtype(kind);

        emit(IR_OP_INTTOPTR, ptr, rval, NULL);

        return ptr;
    }

    qunimpl(a, "Don't know how to convert this to a pointer.");
}

// Return resulting qtemp.
astn convert_integer_type(astn a, ir_type_E t) {
    ir_type_E a_type = ir_type(a);

    if (a_type == t)
        return a;

    bool a_is_signed = type_is_signed[a_type];

    if (a_is_signed && (a_type - t) == 1) // iN to uN
        return a;

    if (!a_is_signed && (t - a_type) == 1) // uN to iN
        return a;

    astn new_t = qtype_alloc(t);
    astn target = qprepare_target(NULL, new_t);

    if (t > a_type) {
        if (a_is_signed) {
            emit(IR_OP_SEXT, target, a, NULL);
        } else {
            emit(IR_OP_ZEXT, target, a, NULL);
        }
    } else {
        qunimpl(a, "Unimplemented: integer truncation");
    }

    return target;
}

// 6.3.1.8  Integer conversions
// Returns new type.
astn do_integer_conversions(astn a, astn b, astn *a_new, astn *b_new) {
    if (!is_integer(a))
        qunimpl(a, "Node passed to do_integer_conversions is not an integer!");

    if (!is_integer(b))
        qunimpl(b, "Node passed to do_integer_conversions is not an integer!");

    ir_type_E a_irt = ir_type(a);
    ir_type_E b_irt = ir_type(b);

    const bool a_is_signed = type_is_signed[a_irt];
    const bool b_is_signed = type_is_signed[b_irt];

    // If both operands have the same type, then no further conversion is needed.

    if (a_irt ==  b_irt) {
        *a_new = a;
        *b_new = b;
        return get_qtype(*a_new);
    }

    // If both operands are signed or both are unsigned, then convert the lesser
    // rank to the greater one's type.

    if ((a_is_signed && b_is_signed) || (!a_is_signed && !b_is_signed)) {
        ir_type_E target_type = a_irt > b_irt ? a_irt : b_irt;
        *a_new = convert_integer_type(a, target_type);
        *b_new = convert_integer_type(b, target_type);
        return get_qtype(*a_new);
    }

    //

    astn signed_one = a_is_signed ? a : b;
    astn unsigned_one = a_is_signed ? b : a;

    ir_type_E signed_one_rank = ir_type(signed_one);
    ir_type_E unsigned_one_rank = ir_type(unsigned_one);

    // If the unsigned operand has rank >= the signed one, then convert the signed
    // one to the unsigned one's type.
    if (unsigned_one_rank > signed_one_rank) {
        ir_type_E target_type = unsigned_one_rank;
        *a_new = convert_integer_type(a, target_type);
        *b_new = convert_integer_type(b, target_type);
        return get_qtype(*a_new);
    }

    // If the type of the signed operand can represent all values of the unsigned
    // operand, convert the unsigned one to the signed one's type.
    if ((signed_one_rank - unsigned_one_rank) > 1) {
        ir_type_E target_type = signed_one_rank;
        *a_new = convert_integer_type(a, target_type);
        *b_new = convert_integer_type(b, target_type);
        return get_qtype(*a_new);
    }
    // Else, convert both to the unsigned type corresponding to the signed operand's
    // type.
    ir_type_E target_type = signed_one_rank - 1;
    *a_new = convert_integer_type(a, target_type);
    *b_new = convert_integer_type(b, target_type);
    return get_qtype(*a_new);
}

// 6.3.1.8  Usual arithmetic conversions
// Return resulting type.
astn do_arithmetic_conversions(astn a, astn b, astn *a_new, astn *b_new) {
    // we don't worry about the floats.
    a = do_integer_promotions(a);
    b = do_integer_promotions(b);

    return do_integer_conversions(a, b, a_new, b_new);
}

// The Sneaky Integer Promotions
astn do_integer_promotions(astn a) {
    if (!is_integer(a))
        return a;

    ir_type_E a_rank = ir_type(a);

    if (a_rank < IR_i32) {
        return(convert_integer_type(a, IR_i32));
    }

    return a;
}
