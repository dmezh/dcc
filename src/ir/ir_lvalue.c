#include "ir_lvalue.h"

#include "ir.h"
#include "ir_loadstore.h"
#include "ir_types.h"
#include "ir_util.h"
#include "symtab.h"

/**
 * Get lvalue.
 */
astn gen_lvalue(astn a) {
    switch (a->type) {
        case ASTN_SYMPTR:;
            astn n = a->Symptr.e->ptr_qtemp;
            return n;

        case ASTN_NUM:
            qerrorl(a, "Expression is not assignable!");

        case ASTN_UNOP:
            return gen_indirection(a);

        case ASTN_QTEMP:
            return a;

        default:
            qunimpl(a, "Unimplemented astn kind for gen_lvalue!");
    }
}

/**
 * Generate indirection lvalue (unop *).
 */
astn gen_indirection(astn a) {
    ast_check(a, ASTN_UNOP, "");
    if (a->Unop.op != '*')
        die("Passed wrong unop type to gen_indirection");


    astn targ_rval = gen_rvalue(a->Unop.target, NULL);

    if (!ir_type_matches(targ_rval, IR_ptr))
        qerrorl(a, "Object to be dereferenced is not a pointer.");

    // if the operand points to a function, the result is a function designator.
    // TODO

    // if it points to an object, the result is an lvalue designating the object.
    return gen_lvalue(targ_rval);
}

/**
 * Convert lvalue to rvalue.
 */
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

        case ASTN_SYMPTR:
            return gen_rvalue(a, target);

        default:
            qunimpl(a, "Unsupported astn type for lvalue_to_rvalue!");
    }
}

