#include "quads_print.h"

#include "ast.h"
#include "charutil.h"
#include "quads.h"
#include "quads_cf.h"
#include "util.h"

#include <stdio.h>

char* quad_op_str[] = {
    [Q_MOV] = "MOV",
    [Q_ADD] = "ADD",
    [Q_SUB] = "SUB",
    [Q_MUL] = "MUL",
    [Q_DIV] = "DIV",
    [Q_MOD] = "MOD",
    [Q_SHL] = "SHL",
    [Q_SHR] = "SHR",
    [Q_BWAND] = "BWAND",
    [Q_BWXOR] = "BWXOR",
    [Q_BWOR] = "BWOR",
    [Q_LOAD] = "LOAD",
    [Q_STORE] = "STORE",
    [Q_LEA] = "LEA",
    [Q_CMP] = "CMP",
    [Q_BREQ] = "BREQ",
    [Q_BRNE] = "BRNE",
    [Q_BRLT] = "BRLT",
    [Q_BRLE] = "BRLE",
    [Q_BRGT] = "BRGT",
    [Q_BRGE] = "BRGE",
    [Q_BR] = "BR",
    [Q_RET] = "RET",
    [Q_ARGBEGIN] = "ARGBEGIN",
    [Q_ARG] = "ARG",
    [Q_CALL] = "CALL",
    [Q_FNSTART] = "FNSTART",
    [Q_NEG] = "NEG",
    [Q_BWNOT] = "BWNOT"
};

void print_node(const_astn qn, FILE* f) {
    switch (qn->type) {
        case ASTN_SYMPTR:   fprintf(f, "%s", qn->Symptr.e->ident); return;
        case ASTN_NUM:      print_number(&qn->Num.number, f); return;
        case ASTN_STRLIT:   
            fprintf(f, "(strlit)\"");
            for (size_t i=0; i<qn->Strlit.strlit.len; i++) {
                emit_char(qn->Strlit.strlit.str[i], f);
            }
            fprintf(f, "\"");
            return;
        case ASTN_QTEMP:    fprintf(f, "%%T%05d.%d", qn->Qtemp.tempno, qn->Qtemp.stack_offset); return;
        case ASTN_QBBNO:    fprintf(f, "BB.%s.%d", qn->Qbbno.bb->fn, qn->Qbbno.bb->bbno); return;
        case ASTN_IDENT:    fprintf(f, "%s", qn->Ident.ident); return;
        case ASTN_IRTYPE:   fprintf(f, "IR type: %d", qn->IRtype.ir_type); return;
        default: eprintf("couldnt't handle node %d\n", qn->type); die("eh");
    }
}

void print_quad(const quad* q, FILE* f) {
    if (q->target) {
        print_node(q->target, f);
        fprintf(f, " = ");
    }
    fprintf(f, "%s ", quad_op_str[q->op]);
    if (q->src1) {
        print_node(q->src1, f);
    }
    if (q->src2) {
        fprintf(f, ", ");
        print_node(q->src2, f);
    }
    fprintf(f, "\n");
}

void print_bb(BB* b, FILE *f) {
    quad* q = b->start;
    while (q) {
        print_quad(q, f);
        q = q->next;
    }
}

void print_bbs(FILE* f) {
    BBL* bbl = &bb_root;
    bbl = bbl_next(bbl); // first me is null
    while (bbl) {
        BB *bb = bbl_data(bbl);
        fprintf(f, "BB.%s.%d:\n", bb->fn, bb->bbno);
        print_bb(bb, f);
        fprintf(f, "\n");
        bbl = bbl_next(bbl);
    }
}
