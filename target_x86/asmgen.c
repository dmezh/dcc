#include "asmgen.h"

#include "../3-quads/quads.h"
#include "../3-quads/quads_cf.h"
#include "../3-quads/quads_print.h"
#include "charutil.h"
#include "semval.h"
#include "symtab.h"
#include "types.h"
#include "util.h"

#include <stdio.h>

FILE* out;

int stack_offset;
int strlit_count = 0;
struct strlit strlits[100]; // can't have more than 100 string literals

const unsigned stack_align = 16; // desired stack alignment

#define ea(...) do { \
    fprintf(out, "\t\t" __VA_ARGS__); \
    fprintf(out, "\n"); \
} while (0);

long get_num(astn *n) {
    return (long)n->astn_num.number.integer;
}

char* g_id(astn *n) {
    return n->astn_ident.ident;
}

void all2XXX(astn *n, char* r) {
    switch (n->type) {
        case ASTN_SYMPTR: ; // empty stmt
            st_entry *e = n->astn_symptr.e;
            //fprintf(stderr, "al2xxx examining %s\n", e->ident);
            switch (e->storspec) {
                case SS_AUTO:
                    ea("movl\t%d(%%ebp), %s", -(e->stack_offset), r);
                    break;
                case SS_EXTERN:
                case SS_STATIC:
                    //todo("static local vars");
                    ea("movl\t%s, %s", e->ident, r);
                    break;
                default:
                    die("invalid storage type for value during asmgen");
                    break;
            }
            break;
        case ASTN_NUM:
            ea("movl\t$%ld, %s", (long)n->astn_num.number.integer, r);
            break;
        case ASTN_QTEMP:
            ea("movl\t%d(%%ebp), %s", -(n->astn_qtemp.stack_offset), r);
            break;
        case ASTN_STRLIT:
            strlits[strlit_count] = n->astn_strlit.strlit;
            ea("movl\t$strl.%d, %s", strlit_count, r);
            strlit_count++;
            break;

        default:
            die("invalid astn during asmgen");
    }
}

void XXX2all(astn *n, char* r) {
    switch (n->type) {
        case ASTN_SYMPTR: ; //empty stmt
            st_entry *e = n->astn_symptr.e;
            switch (e->storspec) {
                case SS_AUTO:
                    ea("movl\t%s, %d(%%ebp)", r, -(e->stack_offset));
                    break;
                case SS_EXTERN:
                case SS_STATIC:
                    ea("movl\t%s, %s", r, e->ident);
                    break;
                default:
                    die("invalid storage type for value during asmgen");
                    break;
            }
            break;
        case ASTN_QTEMP:
            ea("movl\t%s, %d(%%ebp)", r, -(n->astn_qtemp.stack_offset));
            break;
        default:
            die("invalid astn during asmgen");
    }
}

void eax2all(astn *n) { XXX2all(n, "%eax"); }
void edx2all(astn *n) { XXX2all(n, "%edx"); }
void all2eax(astn *n) { all2XXX(n, "%eax"); }
void all2edx(astn *n) { all2XXX(n, "%edx"); }

static int argcount = 0;

void asmgen_q(quad* q) {
    switch (q->op) {
        case Q_FNSTART:
            ea("pushl\t%%ebp");
            ea("movl\t%%esp, %%ebp");
            fprintf(out, "\t\tsubl\t$%d, %%esp\n", stack_offset);
            break;
        case Q_MOV:
            all2eax(q->src1);
            eax2all(q->target);
            break;
        case Q_ADD:
            all2eax(q->src1);
            all2edx(q->src2);
            ea("addl\t%%edx, %%eax");
            eax2all(q->target);
            break;
        case Q_SUB:
            all2eax(q->src1);
            all2edx(q->src2);
            ea("subl\t%%edx, %%eax");
            eax2all(q->target);
            break;
        case Q_MUL:
            all2eax(q->src1);
            all2edx(q->src2);
            ea("\t\timul\t%%edx, %%eax\n");
            eax2all(q->target);
            break;
        case Q_RET:
            if (q->src1) all2eax(q->src1);
            fprintf(out, "\t\tleave\n");
            fprintf(out, "\t\tret\n");
            break;
        case Q_ARGBEGIN:
            argcount = 0;
            break;
        case Q_ARG:
            all2eax(q->src1);
            ea("pushl\t%%eax");
            argcount++;
            break;
        case Q_CALL:
            if (q->src1->type == ASTN_IDENT) { // override default action of all2XXX
                ea("call\t%s", q->src1->astn_ident.ident);
            } else {
                fprintf(stderr, "you're trying to call a non-ident - be my guest but you're probably dead\n");
                all2eax(q->src1);
                ea("call\t%%eax");
            }
            eax2all(q->target);
            ea("addl\t$%d, %%esp", argcount*4);
            argcount = 0;
            break;
        case Q_LEA: // similar breakdown as XXX2all
            switch (q->src1->type) {
                case ASTN_SYMPTR: ; // empty stmt
                    st_entry *e = q->src1->astn_symptr.e;
                    switch (e->storspec) {
                        case SS_AUTO:
                            ea("leal\t%d(%%ebp), %%eax", -(e->stack_offset));
                            break;
                        case SS_EXTERN:
                        case SS_STATIC:
                            ea("leal\t%s, %%eax", e->ident);
                            break;
                        default:
                            die("invalid storage type for leal during asmgen");
                            break;
                    }
                    break;
                case ASTN_QTEMP:
                    ea("leal\t%d(%%ebp), %%eax", -(q->src1->astn_qtemp.stack_offset));
                default:
                    die("invalid astn during asmgen");
            }
            eax2all(q->target);
            break;
        case Q_STORE:
            all2eax(q->src1);
            all2edx(q->target);
            ea("movl\t%%eax, (%%edx)"); // store
            break;
        case Q_LOAD:
            if (q->src1->type != ASTN_SYMPTR && q->src1->type != ASTN_QTEMP)
                die("encountered non-mem source node for load");
            all2eax(q->src1);
            ea("movl\t(%%eax), %%eax"); // load
            eax2all(q->target);
            break;
        case Q_NEG:
            all2eax(q->src1);
            ea("negl\t%%eax");
            eax2all(q->target);
            break;
        case Q_BWNOT:
            all2eax(q->src1);
            ea("notl\t%%eax");
            eax2all(q->target);
            break;
        case Q_CMP:
            all2eax(q->src2);
            all2edx(q->src1);
            ea("cmpl\t%%eax, %%edx");
            break;
        case Q_BR:
            fprintf(out, "\t\tjmp\t"); e_bba(q->src1); fprintf(out, "\n");
            break;
        case Q_BREQ: e_cbr("je",  q);   break;
        case Q_BRNE: e_cbr("jne", q);   break;
        case Q_BRLT: e_cbr("jl",  q);   break;
        case Q_BRLE: e_cbr("jle", q);   break;
        case Q_BRGT: e_cbr("jg",  q);   break;
        case Q_BRGE: e_cbr("jge", q);   break;
        default:
            printf("unhandled quad %s\n", quad_op_str[q->op]);
    }
}

void e_cbr(char *op, quad* q) {
    fprintf(out, "\t\t%s\t", op);
    e_bba(q->src1);
    // uncond jump afterwards for the false branch, could be optimized
    fprintf(out, "\n\t\tjmp\t");
    e_bba(q->src2);
    fprintf(out, "\n");
}
void e_bba(astn *n) { e_bb(n->astn_qbbno.bb); }
void e_bb(BB* b) { fprintf(out, "BB.%s.%d", b->fn, b->bbno); }

void asmgen(BBL* head) {
    // init output file
    out = stdout;
    fprintf(out, "# ASM OUTPUT\n# compiled poorly :)\n\n");

    // init globals, except functions
    st_entry* e = root_symtab.first;
    while (e) {
        if (e->entry_type == STE_VAR) {
            fprintf(out, ".globl %s\n", e->ident);
            fprintf(out, ".comm %s, %d\n", e->ident, get_sizeof(e->type));
        }
        e = e->next;
    }

    BBL *bbl = head;
    if (bbl == &bb_root) bbl = bbl_next(bbl);
    while (bbl) {
        BB* bb = bbl_data(bbl);
        // "declare" function
        if (bb->bbno == 0) { // function start
            fprintf(out, ".globl\t%s\n", bb->fn);
            fprintf(out, "%s:\n", bb->fn);
            stack_offset = bb->stack_offset_ez;
            // round up to stack_align multiple
            stack_offset = ((stack_offset + (stack_align-1))/stack_align)*stack_align;
        }

        // block label
        fprintf(out, "BB.%s.%d:\n", bb->fn, bb->bbno);

        // generate quads for this bb
        quad *q = bb->start;
        while (q) {
            printf("\t# quad "); print_quad(q);
            asmgen_q(q);
            q = q->next;
        }

        printf("\n");
        bbl = bbl_next(bbl);
    }

    fprintf(out, ".section .rodata\n");
    for (int i=0; i<strlit_count; i++) {
        fprintf(out, "strl.%d: .asciz \"", i);
        struct strlit *strl = &strlits[i];
        for (size_t j=0; j<strl->len; j++) {
            emit_char(strl->str[j], out);
        }
        fprintf(out, "\"\n");
    }
}
