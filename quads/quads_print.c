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

void print_node(const astn* qn) {
    switch (qn->type) {
        case ASTN_SYMPTR:   printf("%s", qn->astn_symptr.e->ident); return;
        case ASTN_NUM:      print_number(&qn->astn_num.number, stdout); return;
        case ASTN_STRLIT:   
            printf("(strlit)\"");
            for (size_t i=0; i<qn->astn_strlit.strlit.len; i++) {
                emit_char(qn->astn_strlit.strlit.str[i], stdout);
            }
            printf("\"");
            return;
        case ASTN_QTEMP:    printf("%%T%05d.%d", qn->astn_qtemp.tempno, qn->astn_qtemp.stack_offset); return;
        case ASTN_QBBNO:    printf("BB.%s.%d", qn->astn_qbbno.bb->fn, qn->astn_qbbno.bb->bbno); return;
        case ASTN_IDENT:    printf("%s", qn->astn_ident.ident); return;
        default: fprintf(stderr, "couldnt't handle node %d\n", qn->type); die("eh");
    }
}

void print_quad(quad* q) {
    if (q->target) {
        print_node(q->target);
        printf(" = ");
    }
    printf("%s ", quad_op_str[q->op]);
    if (q->src1) {
        print_node(q->src1);
    }
    if (q->src2) {
        printf(", ");
        print_node(q->src2);
    }
    printf("\n");
}

void print_bb(BB* b) {
    quad* q = b->start;
    while (q) {
        print_quad(q);
        q = q->next;
    }
}

void print_bbs() {
    BBL* bbl = &bb_root;
    bbl = bbl_next(bbl); // first me is null
    while (bbl) {
        BB *bb = bbl_data(bbl);
        printf("BB.%s.%d:\n", bb->fn, bb->bbno);
        print_bb(bb);
        printf("\n");
        bbl = bbl_next(bbl);
    }
}
