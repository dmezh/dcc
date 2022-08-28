#include "ir_arithmetic.h"

#include "ir.h"
#include "ir_print.h"
#include "ir_util.h"
#include "parser.tab.h"


// 6.2.5.18 - Integer and floating types are collectively called arithmetic types.
bool type_is_arithmetic(const_astn a) {
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

        astn l_rval = gen_rvalue(l, NULL);
        astn r_rval = gen_rvalue(r, NULL);

        target = qprepare_target(target, get_qtype(l)); // resultant type

        emit(IR_OP_ADD, target, l_rval, r_rval);
    } else {
        die("Unimplemented or invalid operands in gen_add_rvalue!");
    }


    return target;
}

astn gen_sub_rvalue(astn a, astn target) {
    (void)target;
    qunimpl(a, "Unimplemented: gen_sub_rvalue :(");
}


