#include "ir_print.h"

#include "ir.h"
#include "ir_state.h"

#include "ast.h"
#include "symtab.h"

static FILE *f;

#define qprintf(...)   \
    {                       \
        FILE *o;            \
        if (f) o = f;       \
        else o = stderr;    \
                            \
        fprintf(o, __VA_ARGS__); \
    }


const char *quad_astn_oneword_str(const_astn a) {
    char *ret;

    if (!a) die("gave null astn to quad_print?");

    switch (a->type) {
        case ASTN_LIST:
            a = list_data(a);
            return quad_astn_oneword_str(a);

        case ASTN_QTEMP:
            asprintf(&ret, "%%%d", a->Qtemp.tempno);
            break;

        case ASTN_STRLIT:
            asprintf(&ret, "%s", a->Strlit.strlit.str);
            break;

        case ASTN_QTYPE:
            asprintf(&ret, "%s", ir_type_str[a->Qtype.qtype]);
            break;

        case ASTN_NUM: // needs work for correctness, print numbers as intended
            asprintf(&ret, "%d", (int)a->Num.number.integer);
            break;

        default:
            qunimpl(a, "Unable to get oneword for astn :(");
            return 0;
    }

    return ret;
}

void quad_print(quad first) {
    switch (first->op) {
        case IR_OP_UNKNOWN: die("IR op is UNKNOWN"); break;

        case IR_OP_ALLOCA:
            qprintf("    %s = alloca %s\n", quad_astn_oneword_str(first->target), quad_astn_oneword_str(first->target->Qtemp.qtype));
            break;

        case IR_OP_RETURN:
            if (!first->src1) {
                qprintf("    ret void\n");
            } else {
                qprintf("    ret i32 %s\n", quad_astn_oneword_str(first->src1)); 
            }
            break;

        case IR_OP_ADD:
            qprintf("    %s = add i32 %s, %s\n", quad_astn_oneword_str(first->target), quad_astn_oneword_str(first->src1), quad_astn_oneword_str(first->src2));
            break;

        default:
            die("Unhandled quad in quad_print");
            break;
    }
}

void quads_dump_llvm(FILE *o) {
    f = o;

    // fix for multiple bbs

    qprintf("define %s @%s() {\n", ir_type_str[get_qtype(irst.bb->fn->type->Type.derived.target)->Qtype.qtype], irst.bb->fn->ident);

    quad q = irst.bb->first;
    while (q) {
        quad_print(q);
        q = q->next;
    }

    qprintf("}\n");
}
