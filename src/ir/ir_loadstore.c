#include "ir_loadstore.h"

#include "ast.h"
#include "ir.h"
#include "ir_util.h"
#include "ir_types.h"
#include "lexer.h" // for print_context
#include "util.h"

/**
 * Load and return value.
 */
astn gen_load(astn a, astn target) {
    // assuming local non-parameter variables right now.
    astn addr = a;

    switch (a->type) {
        case ASTN_SYMPTR:
            die("");
            {
                astn t = a->Symptr.e->type;
                if (t->Type.is_derived && t->Type.derived.type == t_ARRAY) {
                    astn ptr_type = qtype_alloc(IR_ptr);
                    // say addr has QTYPE [10 x i32]. ptr should have qtype ptr to i32.
                    astn targ_of_array = t->Type.derived.target;

                    ptr_type->Qtype.derived_type = targ_of_array;

                    astn ptr = new_qtemp(ptr_type);
                    emit4(IR_OP_GEP, ptr, addr, simple_constant_alloc(0), simple_constant_alloc(0));
                    return ptr;
                }
            }
            break;

        case ASTN_UNOP:
            die("");
            break;

        case ASTN_QTEMP:
            break;

        default:
            qunimpl(a, "Bizarre type to try to load...");
    }

    if (get_qtype(addr)->Qtype.qtype != IR_ptr) {
       qunimpl(addr, "Dereferenced non-pointer object!");
    }

    target = qprepare_target(target, get_qtype(get_qtype(addr)->Qtype.derived_type));

    // may need revision for globals
    ast_check(addr, ASTN_QTEMP, "Expected qtemp for loading!");
    emit(IR_OP_LOAD, target, addr, NULL);
    return target;
}

/*
 * Return address/object to write to.
 */
astn gen_lvalue(astn a) {
    switch (a->type) {
        case ASTN_SYMPTR:;
            astn n = a->Symptr.e->ptr_qtemp;
            //n->Qtemp.qtype = n->Qtemp.qtype->Qtype.derived_type;
            return n;

        case ASTN_NUM:
            qprintcontext(a->context);
            qerror("Expression is not assignable!");

        case ASTN_UNOP:;
            if (a->Unop.op != '*')
                qunimpl(a, "Unimplemented unop for gen_lvalue!");

            astn l = gen_rvalue(a->Unop.target, NULL); // get the rvalue of the target
            return l;

        case ASTN_QTEMP:
            return gen_rvalue(a, NULL);

        default:
            qunimpl(a, "Unimplemented astn kind for gen_lvalue!");
    }
}

astn gen_store(astn target, astn val) {
    astn lval = gen_lvalue(target);
    astn rval = gen_rvalue(val, NULL);

    emit(IR_OP_STORE, lval, rval, NULL);

    return rval;
}

/*
 * Generate code for ASTN_ASSIGN. We return the rvalue
 * (right side).
 */
astn gen_assign(astn a) {
    ast_check(a, ASTN_ASSIGN, "");

    return gen_store(a->Assign.left, a->Assign.right);
}
