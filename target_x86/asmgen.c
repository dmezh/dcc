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

#define ea(...) do { \
    fprintf(out, "\t\t" __VA_ARGS__); \
    fprintf(out, "\n"); \
} while (0);

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

char* all2XXX(astn *n, char* r) {
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
    return r;
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

void eax2all(astn *n) {
    XXX2all(n, "%eax");
}

void edx2all(astn *n) {
    XXX2all(n, "%edx");
}

char* all2eax(astn *n) {
    return all2XXX(n, "%eax");
}

char* all2edx(astn *n) {
    return all2XXX(n, "%edx");
}

// SUB, ADD
void asm_msa(quad* q, char* opc) {
    int src1_o, src2_o, trg_o;
    bool src1m=getoffset(q->src1, &src1_o);
    bool src2m=getoffset(q->src2, &src2_o);
    bool trgm=getoffset(q->target, &trg_o);

    if (!trgm) die("invalid ADD/SUB target");
    // put src1 in eax
    all2eax(q->src1);
    // perform add
    fprintf(out, "\t\t%s\t%d(%%ebp), %%eax\n", opc, src2_o);
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
            all2eax(q->src1);
            eax2all(q->target);
            break;
        case Q_ADD:
            asm_msa(q, "addl");
            break;
        case Q_SUB:
            asm_msa(q, "subl");
            break;
        case Q_MUL:
            if (!trgm) die("invalid MUL target");
            all2eax(q->src1);
            all2edx(q->src2);
            ea("\t\timul\t%%edx, %%eax\n");
            eax2m(trg_o);
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

    //fprintf(out, ".section rodata\n");
    // init globals, except functions
    st_entry* e = root_symtab.first;
    while (e) {
        if (e->entry_type == STE_VAR) {
            fprintf(out, ".globl %s\n", e->ident);
            fprintf(out, ".comm %s, %d\n", e->ident, get_sizeof(e->type));
        }
        e = e->next;
    }

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
