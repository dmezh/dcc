#include "ir_print.h"

#include "ir.h"
#include "ir_state.h" // to be removed
#include "ir_types.h"
#include "ir_util.h"

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

void quad_print_blankline(void) {
    qprintf("\n");
}

const char *qonewordt(astn a) {
    char *ret;
    asprintf(&ret, "%s %s", qoneword(get_qtype(a)), qoneword(a));
    return ret;
}

const char *qoneword(astn a) {
    char *ret;

    if (!a) die("gave null astn to quad_print?");

    switch (a->type) {
        case ASTN_LIST:
            a = list_data(a);
            return qoneword(a);

        case ASTN_QTEMP:
            if (a->Qtemp.name)
                asprintf(&ret, "@%s", a->Qtemp.name);
            else
                asprintf(&ret, "%%%d", a->Qtemp.tempno);
            break;

        case ASTN_STRLIT:
            asprintf(&ret, "%s", a->Strlit.strlit.str);
            break;

        case ASTN_QTYPE:
            if (ir_type_matches(a, IR_arr)) {
                // we're going to stop at the first non-array type.
                ast_check(a->Qtype.derived_type, ASTN_TYPE, "");
                if (!a->Qtype.derived_type->Type.derived.size)
                    die("Expected array to have size.");
                if (!a->Qtype.derived_type->Type.derived.target)
                    die("Expected array to have target.");

                asprintf(&ret, "[%s x %s]", qoneword(ir_dtype(a)->Type.derived.size), qoneword(get_qtype(ir_dtype(a)->Type.derived.target)));
            } else {
                asprintf(&ret, "%s", ir_type_str[ir_type(a)]);
            }
            break;

        case ASTN_NUM: // needs work for correctness, print numbers as intended
            asprintf(&ret, "%d", (int)a->Num.number.integer);
            break;

        case ASTN_TYPE:
            return qoneword(get_qtype(a));

        case ASTN_SYMPTR:
            return a->Symptr.e->ident;

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
            qprintf("    %s = alloca %s\n",
                    qoneword(first->target),
                    qoneword(ir_dtype(first->target)));
            break;

        case IR_OP_RETURN:
            if (!first->src1) {
                qprintf("    ret void\n");
            } else {
                qprintf("    ret i32 %s\n",
                        qoneword(first->src1));
            }
            break;

        case IR_OP_LOAD:
            qprintf("    %s = load %s, %s\n",
                    qoneword(first->target),
                    qoneword(get_qtype(first->target)),
                    qonewordt(first->src1));
            break;

        case IR_OP_STORE:
            ast_check(first->target, ASTN_QTEMP, "");
            qprintf("    store %s %s, %s\n",
                    qoneword(get_qtype(ir_dtype(first->target))),
                    qoneword(first->src1),
                    qonewordt(first->target));
            break;

        case IR_OP_ADD:
            qprintf("    %s = add %s %s, %s\n",
                    qoneword(first->target),
                    qoneword(qtype_alloc(ir_type(first->target))),
                    qoneword(first->src1),
                    qoneword(first->src2));
            break;

        case IR_OP_SUB:
            qprintf("    %s = sub %s %s, %s\n",
                    qoneword(first->target),
                    qoneword(qtype_alloc(ir_type(first->target))),
                    qoneword(first->src1),
                    qoneword(first->src2));
            break;

        case IR_OP_GEP:
            qprintf("    %s = getelementptr %s, %s, %s",
                    qoneword(first->target),
                    qoneword(ir_dtype(first->src1)),
                    qonewordt(first->src1),
                    qonewordt(first->src2));

            if (first->src3)
                qprintf(", %s", qonewordt(first->src3));

            qprintf("\n");

            break;

        case IR_OP_DEFGLOBAL:
            qprintf("%s = global %s zeroinitializer\n",
                    qoneword(first->target),
                    qoneword(ir_dtype(first->target)));
            break;

        case IR_OP_SEXT:
            qprintf("    %s = sext %s to %s\n",
                    qoneword(first->target),
                    qonewordt(first->src1),
                    qoneword(qtype_alloc(ir_type(first->target))));
            break;

        case IR_OP_ZEXT:
            qprintf("    %s = zext %s to %s\n",
                    qoneword(first->target),
                    qonewordt(first->src1),
                    qoneword(qtype_alloc(ir_type(first->target))));
            break;

        default:
            die("Unhandled quad in quad_print");
            break;
    }
}

void quads_dump_llvm(FILE *o) {
    f = o;

    // fix for multiple bbs

    quad g = bbl_this(irst.root_bbl)->first;
    while (g) {
        quad_print(g);
        g = g->next;
    }

    irst.bb = bbl_this(bbl_next(irst.root_bbl));

    qprintf("define %s @%s() {\n", ir_type_str[ir_type(irst.bb->fn->type->Type.derived.target)], irst.bb->fn->ident);

    quad q = irst.bb->first;
    while (q) {
        quad_print(q);
        q = q->next;
    }

    qprintf("}\n");
}
