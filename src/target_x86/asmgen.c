#include "asmgen.h"

#include "main.h"
#include "quads.h"
#include "quads_cf.h"
#include "quads_print.h"
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

static const char const * argcount_to_reg[] = {"RDI", "RSI", "RDX", "RCX", "R8", "R9"};

#define ea(...) { \
    fprintf(out, "\t\t" __VA_ARGS__); \
    fprintf(out, "\n"); \
}

void all2XXX(astn n, char* r) {
    switch (n->type) {
        case ASTN_SYMPTR: ; // empty stmt
            sym e = n->Symptr.e;
            //eprintf("al2xxx examining %s\n", e->ident);
            switch (e->storspec) {
                case SS_AUTO:
                    ea("movq\t%d(%%rbp), %s", -(e->stack_offset), r);
                    // fprintf(stderr, "stack offset of %s: %d\n", e->ident, e->stack_offset);
                    break;
                case SS_EXTERN:
                case SS_STATIC:
                    //todo("static local vars");
                    ea("movq\t%s(%%rip), %s", e->ident, r);
                    break;
                default:
                    die("invalid storage type for value during asmgen");
                    break;
            }
            break;
        case ASTN_NUM:
            ea("movq\t$%ld, %s", (long)n->Num.number.integer, r);
            break;
        case ASTN_QTEMP:
            ea("movq\t%d(%%rbp), %s", -(n->Qtemp.stack_offset), r);
            break;
        case ASTN_STRLIT:
            strlits[strlit_count] = n->Strlit.strlit;
            ea("leaq\tstrl.%d(%%rip), %s", strlit_count, r);
            strlit_count++;
            break;

        default:
            die("invalid astn during asmgen");
    }
}

void XXX2all(astn n, char* r) {
    switch (n->type) {
        case ASTN_SYMPTR: ; //empty stmt
            sym e = n->Symptr.e;
            switch (e->storspec) {
                case SS_AUTO:
                    ea("movq\t%s, %d(%%rbp)", r, -(e->stack_offset));
                    break;
                case SS_EXTERN:
                case SS_STATIC:
                    ea("movq\t%s(%%rip), %s", r, e->ident);
                    break;
                default:
                    die("invalid storage type for value during asmgen");
                    break;
            }
            break;
        case ASTN_QTEMP:
            ea("movq\t%s, %d(%%rbp)", r, -(n->Qtemp.stack_offset));
            break;
        default:
            die("invalid astn during asmgen");
    }
}

void rax2all(astn n) { XXX2all(n, "%rax"); }
void rcx2all(astn n) { XXX2all(n, "%rcx"); }
void rdx2all(astn n) { XXX2all(n, "%rdx"); }
void all2rax(astn n) { all2XXX(n, "%rax"); }
void all2rcx(astn n) { all2XXX(n, "%rcx"); }
void all2rdx(astn n) { all2XXX(n, "%rdx"); }

static int argcount = 0;

void asmgen_q(quad* q) {
    switch (q->op) {
        case Q_FNSTART:
            ea("pushq\t%%rbp");
            ea("movq\t%%rsp, %%rbp");
            fprintf(out, "\t\tsubq\t$%d, %%rsp\n", stack_offset);
            for (int i = 0; i < q->src1->Symptr.e->fn_scope->param_count; i++) {
                ea("movq\t%%%s, -%d(%%rbp)", argcount_to_reg[i], (i+1)*8);
            }
            break;
        case Q_MOV:
            all2rax(q->src1);
            rax2all(q->target);
            break;
        case Q_ADD:
            all2rax(q->src1);
            all2rdx(q->src2);
            ea("addq\t%%rdx, %%rax");
            rax2all(q->target);
            break;
        case Q_SUB:
            all2rax(q->src1);
            all2rdx(q->src2);
            ea("subq\t%%rdx, %%rax");
            rax2all(q->target);
            break;
        case Q_MUL:
            all2rax(q->src1);
            all2rdx(q->src2);
            ea("imulq\t%%rdx, %%rax");
            rax2all(q->target);
            break;
        case Q_DIV:
            all2rax(q->src1);
            ea("cdq"); // now rdx:rax is src1
            all2rcx(q->src2);
            ea("idiv\t%%rcx");
            rax2all(q->target); // give result
            break;
        case Q_MOD:
            all2rax(q->src1);
            ea("cdq"); // now rdx:rax is src1
            all2rcx(q->src2);
            ea("idiv\t%%rcx");
            rdx2all(q->target); // give remainder
            break;
        case Q_RET:
            if (q->src1) all2rax(q->src1);
            fprintf(out, "\t\tleave\n");
            fprintf(out, "\t\tret\n");
            break;
        case Q_ARGBEGIN:
            argcount = 0;
            break;
        case Q_ARG:
            if (argcount > 5) {
                die("I can't handle more than 6 args yet :(\n");
            }
            all2rax(q->src1);
            ea("movq\t%%rax, %%%s", argcount_to_reg[argcount]);
            argcount++;
            break;
        case Q_CALL:
            if (q->src1->type == ASTN_IDENT) { // override default action of all2XXX
                if (dcc_is_host_darwin()) {
                    ea("call\t_%s", q->src1->Ident.ident);
                } else {
                    ea("call\t%s", q->src1->Ident.ident);
                }
            } else {
                eprintf("you're trying to call a non-ident - be my guest but you're probably dead\n");
                all2rax(q->src1);
                ea("call\t%%rax");
            }
            rax2all(q->target);
            // ea("addq\t$%d, %%rsp", argcount*8);
            argcount = 0;
            break;
        case Q_LEA: // similar breakdown as XXX2all
            switch (q->src1->type) {
                case ASTN_SYMPTR: ; // empty stmt
                    sym e = q->src1->Symptr.e;
                    switch (e->storspec) {
                        case SS_AUTO:
                            ea("leaq\t%d(%%rbp), %%rax", -(e->stack_offset));
                            break;
                        case SS_EXTERN:
                        case SS_STATIC:
                            ea("leaq\t%s, %%rax", e->ident);
                            break;
                        default:
                            die("invalid storage type for leal during asmgen");
                            break;
                    }
                    break;
                case ASTN_QTEMP:
                    ea("leaq\t%d(%%rbp), %%rax", -(q->src1->Qtemp.stack_offset));
                    break;
                default:
                    die("invalid astn during asmgen");
            }
            rax2all(q->target);
            break;
        case Q_STORE:
            all2rax(q->src1);
            all2rdx(q->target);
            ea("movq\t%%rax, (%%rdx)"); // store
            break;
        case Q_LOAD:
            if (q->src1->type != ASTN_SYMPTR && q->src1->type != ASTN_QTEMP)
                die("encountered non-mem source node for load");
            all2rax(q->src1);
            ea("movq\t(%%rax), %%rax"); // load
            rax2all(q->target);
            break;
        case Q_NEG:
            all2rax(q->src1);
            ea("negq\t%%rax");
            rax2all(q->target);
            break;
        case Q_BWNOT:
            all2rax(q->src1);
            ea("notq\t%%rax");
            rax2all(q->target);
            break;
        case Q_CMP:
            all2rax(q->src2);
            all2rdx(q->src1);
            ea("cmpq\t%%rax, %%rdx");
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

void e_cbr(const char *op, quad* q) {
    fprintf(out, "\t\t%s\t", op);
    e_bba(q->src1);
    // uncond jump afterwards for the false branch, could be optimized
    fprintf(out, "\n\t\tjmp\t");
    e_bba(q->src2);
    fprintf(out, "\n");
}
void e_bba(const_astn n) { e_bb(n->Qbbno.bb); }
void e_bb(const BB* b) { fprintf(out, "BB.%s.%d", b->fn, b->bbno); }

void asmgen(const BBL* head, FILE* f) {
    // init output file
    out = f;
    fprintf(out, "# ASM OUTPUT\n# compiled poorly :)\n\n");

    // init globals, except functions
    sym e = root_symtab.first;
    while (e) {
        if (e->entry_type == STE_VAR) {
            if (e->storspec == SS_STATIC) {
                if (e->linkage == L_EXTERNAL)
                    fprintf(out, ".globl %s\n", e->ident);
                else if (e->linkage == L_INTERNAL)
                    fprintf(out, ".local %s\n", e->ident);

                fprintf(out, ".comm %s, %d\n", e->ident, get_sizeof(e->type));
            } else {
                fprintf(out, ".extern %s\n", e->ident);
            }
        }
        e = e->next;
    }

    const BBL *bbl = head;
    if (bbl == &bb_root) bbl = bbl_next(bbl);
    while (bbl) {
        BB* bb = bbl_data(bbl);
        // "declare" function
        if (bb->bbno == 0) { // function start
            if (dcc_is_host_darwin()) {
                fprintf(out, ".globl\t_%s\n", bb->fn);
                fprintf(out, "_%s:\n", bb->fn);
            } else {
                fprintf(out, ".globl\t%s\n", bb->fn);
                fprintf(out, "%s:\n", bb->fn);
            }
            stack_offset = bb->stack_offset_ez;
            // round up to stack_align multiple
            stack_offset = ((stack_offset + (stack_align-1))/stack_align)*stack_align;
        }

        // block label
        fprintf(out, "BB.%s.%d:\n", bb->fn, bb->bbno);

        // generate quads for this bb
        quad *q = bb->start;
        while (q) {
            fprintf(f, "\t# quad "); print_quad(q, f);
            asmgen_q(q);
            q = q->next;
        }

        fprintf(out, "\n");
        bbl = bbl_next(bbl);
    }

    // fprintf(out, ".section .rodata\n");
    for (int i=0; i<strlit_count; i++) {
        fprintf(out, "strl.%d: .asciz \"", i);
        struct strlit *strl = &strlits[i];
        for (size_t j=0; j<strl->len; j++) {
            emit_char(strl->str[j], out);
        }
        fprintf(out, "\"\n");
    }
}
