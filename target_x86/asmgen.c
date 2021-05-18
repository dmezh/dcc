#include "asmgen.h"

#include "../3-quads/quads.h"
#include "../3-quads/quads_cf.h"
#include "../3-quads/quads_print.h"
#include "util.h"

#include <stdio.h>

FILE* out;

int stack_offset;

const unsigned stack_align = 16;

#define es(op) do { \
    fprintf(out, "\t\t%s\n", #op); \
} while (0)

#define e1(op, l) do { \
    fprintf(out, "\t\t%s\n", #op "\t" #l); \
} while(0)

#define e2(op, l, r) do { \
    fprintf(out, "\t\t%s\n", #op "\t" #l ", " #r); \
} while (0)

char * adir_str[] = {
    [NONE] = "ERROR_NONE",
    [EBP] = "%ebp",
    [ESP] = "%esp",
    [PUSHL] = "pushl",
    [MOVL] = "movl",
    [SUBL] = "subl",
    [LEAVE] = "leave"
};

void aemit(char *dir, char *left, char *right) {
    if (!dir) die("passed NULL opcode directive to asm emitter");
    fprintf(out, "%s", dir);
    if (left)  fprintf(out, "\t\t%s", left);
    if (right) fprintf(out, ", %s", right);
}

bool getoffset(astn *n, int *o) {
    if (n) {
        if (n->type == ASTN_QTEMP) {
            *o = -(n->astn_qtemp.stack_offset);
            return true;
        } else if (n->type == ASTN_SYMPTR) {
            *o = -(n->astn_symptr.e->stack_offset);
            return true;
        }
        return false;
    }
    return false;
}

void m2eax(int off) {
    fprintf(out, "\t\tmovl\t%d(%%ebp), %%eax\n", (off));
}

void eax2m(int off) {
    fprintf(out, "\t\tmovl\t%%eax, %d(%%ebp)\n", (off));
}

void n2eax(astn *n) {
    fprintf(out, "\t\tmovl\t$%ld, %%eax\n", (long)n->astn_num.number.integer);
}

void n2edx(astn *n) {
    fprintf(out, "\t\tmovl\t$%ld, %%edx\n", (long)n->astn_num.number.integer);
}

long get_num(astn *n) {
    return (long)n->astn_num.number.integer;
}

// SUB, ADD
void asm_msa(quad* q, char* opc) {
    int src1_o, src2_o, trg_o;
    bool src1m=getoffset(q->src1, &src1_o);
    bool src2m=getoffset(q->src2, &src2_o);
    bool trgm=getoffset(q->target, &trg_o);

    if (!trgm) die("invalid ADD/SUB target");
    // put src1 in eax
    if (src1m)
        m2eax(src1_o);
    else
        n2eax(q->src1);
    // perform add
    if (src2m)
        fprintf(out, "\t\t%s\t%d(%%ebp), %%eax\n", opc, src2_o);
    else
        fprintf(out, "\t\t%s\t$%ld, %%eax\n", opc, get_num(q->src2));
    // eax has the result now; save it back to the destination
    eax2m(trg_o);
}

static int argcount = 0;

void asmgen_q(quad* q) {
    //fprintf(out, "\t\t");
    int src1_o, src2_o, trg_o;
    bool src1m=getoffset(q->src1, &src1_o);
    bool src2m=getoffset(q->src2, &src2_o);
    bool trgm=getoffset(q->target, &trg_o);

    switch (q->op) {
        case Q_FNSTART:
            e1(pushl, %ebp);
            e2(movl, %esp, %ebp);
            fprintf(out, "\t\tsubl\t$%d, %%esp\n", stack_offset);
            break;
        case Q_MOV:
            if (!trgm) die("invalid MOV target");
            if (src1m) {
                m2eax(src1_o);
                eax2m(trg_o);
            } else {
                fprintf(out, "\t\tmovl\t$%ld, %d(%%ebp)\n", get_num(q->src1), trg_o);
            }
            break;
        case Q_ADD:
            asm_msa(q, "addl");
            break;
        case Q_SUB:
            asm_msa(q, "subl");
            break;
        case Q_MUL:
            if (!trgm) die("invalid MUL target");
            // put src1 in eax
            if (src1m)
                m2eax(src1_o);
            else
                n2eax(q->src1);
            // perform add
            if (src2m)
                fprintf(out, "\t\timul\t%d(%%ebp), %%eax\n", src2_o);
            else {
                n2edx(q->src2);
                fprintf(out, "\t\timul\t%%edx, %%eax\n");
            }
            // eax has the result now; save it back to the destination
            eax2m(trg_o);
            break;
        case Q_RET:
            if (q->src1) {
                if (src1m)
                    m2eax(src1_o);
                else
                    n2eax(q->src1);
            }
            fprintf(out, "\t\tleave\n");
            fprintf(out, "\t\tret\n");
            break;
        case Q_ARGBEGIN:
            argcount = 0;
            break;
        case Q_ARG:
            if (src1m)
                fprintf(out, "\t\tpushl\t%d(%%ebp)\n", src1_o);
            else
                fprintf(out, "\t\tpushl\t$%ld\n", get_num(q->src1));
            argcount++;
            break;
        case Q_CALL:
            if (src1m) { // memory
                fprintf(stderr, "you're trying to call a non-ident - be my guest but you're probably dead\n");
                fprintf(out, "\t\tcall\t%d(%%ebp)\n", src1_o);
            } else if (q->src1->type == ASTN_IDENT) { // ident
                fprintf(out, "\t\tcall\t%s\n", q->src1->astn_ident.ident);
            } else { // constant
                fprintf(out, "\t\tcall\t$%ld\n", get_num(q->src1));
            }
            fprintf(out, "\t\tmovl\t%%eax, %d(%%ebp)\n", trg_o);
            fprintf(out, "\t\taddl\t$%d, %%esp\n", argcount*4);
            argcount = 0;
            break;
        case Q_LEA:
            if (src1m) {
                fprintf(out, "\t\tleal\t%d(%%ebp), %%eax\n", src1_o);
                fprintf(out, "\t\tmovl\t%%eax, %d(%%ebp)\n", trg_o);
            } else {
                die("non-mem type input node for LEA");
            }
            break;
        case Q_STORE: // a mov or so
            if (src1m) {
                m2eax(src1_o);
                fprintf(out, "\t\tmovl\t%d(%%ebp), %%edx\n", trg_o);
                fprintf(out, "\t\tmovl\t%%eax, (%%edx)\n");
            } else {
                fprintf(out, "\t\tmovl\t%d(%%ebp), %%eax\n", trg_o); // get target
                fprintf(out, "\t\tmovl\t$%ld, (%%eax)\n", get_num(q->src1));
            }
            break;
        case Q_LOAD:
            if (src1m) {
                m2eax(src1_o);
                fprintf(out, "\t\tmovl\t(%%eax), %%eax\n"); // load
                fprintf(out, "\t\tmovl\t%%eax, %d(%%ebp)\n", trg_o);
            } else {
                die("load into nonmem");
            }
            break;
        default:
            printf("unhandled quad %s\n", quad_op_str[q->op]);
    }
    //fprintf(out, "\n");
}

void asmgen() {
    // init output file
    out = stdout;
    fprintf(out, "# ASM OUTPUT\n# compiled poorly :)\n\n");

    // init globals, except functions

    BBL *bbl = &bb_root;
    bbl = bbl_next(bbl);

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

        quad *q = bb->start;
        while (q) {
            printf("\t# quad "); print_quad(q);
            asmgen_q(q);
            q = q->next;
        }

        // stuff
        printf("\n");

        bbl = bbl_next(bbl);
    }
}
