#include "ir_print.h"

#include "ir.h"
#include "ir_state.h" // to be removed
#include "ir_types.h"
#include "ir_util.h"

#include "ast.h"
#include "charutil.h"
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
    if (ir_type_matches(a, IR_fn))
        asprintf(&ret, "%s %s", qoneword(ir_dtype(a)->Type.derived.target), qoneword(a));
    else
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
            if (a->Qtemp.name && ir_type_matches(a, IR_struct)) {
                asprintf(&ret, "%%%s", a->Qtemp.name);
            } else if (a->Qtemp.name) {
                asprintf(&ret, "@%s", a->Qtemp.name);
            } else {
                if (a->Qtemp.tempno < 0)
                    qunimpl(a, "Tried to print a negative qtemp number!");
                asprintf(&ret, "%%%d", a->Qtemp.tempno);
            }
            break;

        case ASTN_STRLIT:
            asprintf(&ret, "c\"");

            for (size_t i = 0; i < a->Strlit.strlit.len; i++) {
                char * old_ret = ret;
                asprintf(&ret, "%s%s", ret, get_char_hexesc(a->Strlit.strlit.str[i]));
                free(old_ret);
            }

            asprintf(&ret, "%s\\00\"", ret);
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
            } else if (ir_type_matches(a, IR_struct)) {
                ast_check(ir_dtype(a), ASTN_TYPE, "");
                return qoneword(ir_dtype(a)->Type.tagtype.symbol->qptr);
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
            asprintf(&ret, "@%s", a->Symptr.e->ident);
            break;

        case ASTN_DECLREC:
            asprintf(&ret, "@%s", a->Declrec.e->ident);
            break;

        case ASTN_QBB:
            asprintf(&ret, "%%%s", a->Qbb.bb->name);
            break;

        case ASTN_QTYPECONTAINER:
            return qoneword(a->Qtypecontainer.qtype);

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

        case IR_OP_MUL:
            qprintf("    %s = mul %s %s, %s\n",
                    qoneword(first->target),
                    qoneword(qtype_alloc(ir_type(first->target))),
                    qoneword(first->src1),
                    qoneword(first->src2));
            break;

        case IR_OP_SDIV:
            qprintf("    %s = sdiv %s %s, %s\n",
                    qoneword(first->target),
                    qoneword(qtype_alloc(ir_type(first->target))),
                    qoneword(first->src1),
                    qoneword(first->src2));
            break;

        case IR_OP_UDIV:
            qprintf("    %s = udiv %s %s, %s\n",
                    qoneword(first->target),
                    qoneword(qtype_alloc(ir_type(first->target))),
                    qoneword(first->src1),
                    qoneword(first->src2));
            break;

        case IR_OP_SMOD:
            qprintf("    %s = srem %s %s, %s\n",
                    qoneword(first->target),
                    qoneword(qtype_alloc(ir_type(first->target))),
                    qoneword(first->src1),
                    qoneword(first->src2));
            break;

        case IR_OP_UMOD:
            qprintf("    %s = urem %s %s, %s\n",
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
            if (ir_type_matches(first->target, IR_fn)) {
                ast_check(first->target->Qtemp.global, ASTN_SYMPTR, "");

                sym fn = first->target->Qtemp.global->Symptr.e;
                if (fn->fn_defined) break;

                if (fn->linkage == L_INTERNAL)
                    qerrorl(fn->type, "static function never defined");

                qprintf("declare %s(", qonewordt(first->target));

                astn param = fn->param_list_q;

                while (param) {
                    qprintf("%s", qonewordt(list_data(param)));

                    param = list_next(param);

                    if (param) qprintf(", ");
                }

                qprintf(")\n");
            } else if (ir_type_matches(first->target, IR_struct)) {
                qprintf("%s = type { ",
                        qoneword(first->target));

                sym m = ir_dtype(first->target)->Type.tagtype.symbol->members->first;
                while (m) {
                    qprintf("%s", qoneword(get_qtype(m->type)));
                    m = m->next;

                    if (m)
                        qprintf(", ");
                }

                qprintf(" }\n");
            } else {
                qprintf("%s = ", qoneword(first->target));

                if (first->target->Qtemp.global->type == ASTN_STRLIT) {
                    qprintf("private constant ");
                } else if (*first->target->Qtemp.name == '.') {
                    qprintf("private global ");
                } else {
                    qprintf("global ");
                }

                qprintf("%s ", qoneword(ir_dtype(first->target)));

                if (first->src1) {
                    qprintf("%s", qoneword(first->src1));
                }
                else {
                    qprintf("zeroinitializer");
                }

                qprintf("\n");
            }
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

        case IR_OP_TRUNC:
            qprintf("    %s = trunc %s to %s\n",
                    qoneword(first->target),
                    qonewordt(first->src1),
                    qoneword(get_qtype(first->target)));

            break;

        case IR_OP_INTTOPTR:
            qprintf("    %s = inttoptr %s to %s\n",
                    qoneword(first->target),
                    qonewordt(first->src1),
                    qoneword(qtype_alloc(ir_type(first->target))));

            break;

        case IR_OP_PTRTOINT:
            qprintf("    %s = ptrtoint %s to %s\n",
                    qoneword(first->target),
                    qonewordt(first->src1),
                    qoneword(qtype_alloc(ir_type(first->target))));

            break;

        case IR_OP_FNCALL:;
            ir_type_E fn_ret = ir_type(ir_dtype(first->src1)->Type.derived.target);
            if (fn_ret == IR_void) {
                qprintf("    call %s(",
                        qonewordt(first->src1));
            } else {
                qprintf("    %s = call %s(",
                        qoneword(first->target),
                        qonewordt(first->src1));
            }

            astn arg = first->src2;

            while (arg && list_data(arg)) {
                qprintf("%s", qonewordt(list_data(arg)));
                arg = list_next(arg);
                if (arg && list_data(arg))
                    qprintf(", ");
            }

            qprintf(")\n");

            break;

        case IR_OP_BR: // unconditional branch
            ast_check(first->target, ASTN_QBB, "");
            qprintf("    br label %%%s\n", first->target->Qbb.bb->name);
            break;

        case IR_OP_CONDBR:
            qprintf("    br %s, label %%%s, label %%%s\n",
                    qonewordt(first->target),
                    first->src1->Qbb.bb->name,
                    first->src2->Qbb.bb->name);
            break;

        case IR_OP_CMPEQ:
            qprintf("    %s = icmp eq %s, %s\n",
                    qoneword(first->target),
                    qonewordt(first->src1),
                    qoneword(first->src2));
            break;

        case IR_OP_CMPNE:
            qprintf("    %s = icmp ne %s, %s\n",
                    qoneword(first->target),
                    qonewordt(first->src1),
                    qoneword(first->src2));
            break;

        case IR_OP_CMPLT:
            qprintf("    %s = icmp slt %s, %s\n",
                    qoneword(first->target),
                    qonewordt(first->src1),
                    qoneword(first->src2));
            break;

        case IR_OP_CMPLTEQ:
            qprintf("   %s = icmp sle %s, %s\n",
                    qoneword(first->target),
                    qonewordt(first->src1),
                    qoneword(first->src2));
            break;

        case IR_OP_SWITCHBEGIN:
            qprintf("    switch %s, %s [\n",
                    qonewordt(first->src1),
                    qonewordt(first->target));

            break;

        case IR_OP_SWITCHCASE:
            qprintf("        %s, %s\n",
                    qonewordt(first->target),
                    qonewordt(first->src1));
            break;

        case IR_OP_SWITCHEND:
            qprintf("    ]\n");
            break;
        default:
            die("Unhandled quad in quad_print");
            break;
    }
}

void quads_dump_llvm(FILE *o) {
    f = o;

    // generate anons


    BBL bbl = irst.root_bbl;
    while (bbl) {           // for each function
        BB bb = bbl->me;    // for each basic block
        if (bbl != irst.root_bbl) {
            qprintf("define %s(", qonewordt(symptr_alloc(bb->fn)));
            astn p = bb->fn->param_list_q;

            while (p) {
                astn e = list_data(p);

                if (e->type == ASTN_ELLIPSIS)
                    qprintf("...")
                else
                    qprintf("%s", qonewordt(e));

                p = list_next(p);
                if (p)
                    qprintf(", ");
            }

            qprintf(") {\n");
        }

        while (bb) {        // for each quad
            if (bb->name)
                qprintf("%s:\n", bb->name);

            quad g = bb->first;
            while (g) {
                quad_print(g);
                g = g->next;
            }

            bb = bb->next;
        }

        if (bbl != irst.root_bbl)
            qprintf("}\n");

        bbl = bbl->next;
    }
}
