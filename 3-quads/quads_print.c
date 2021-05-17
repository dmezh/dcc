#include "quads_print.h"

#include "ast.h"
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
    [Q_BR] = "BR"
};

void print_node(const astn* qn) {
    switch (qn->type) {
        case ASTN_SYMPTR:   printf("%s", qn->astn_symptr.e->ident); return;
        case ASTN_NUM:      print_number(&qn->astn_num.number); return;
        case ASTN_STRLIT:   printf("(strlit)\"%s\"", qn->astn_strlit.strlit.str); return;
        case ASTN_QTEMP:    printf("%%T%05d", qn->astn_qtemp.tempno); return;
        case ASTN_QBBNO:    printf("BB.%s.%d", qn->astn_qbbno.bb->fn, qn->astn_qbbno.bb->bbno); return;
        default: die("eh");
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

/*
int a[10];
int f()
{
int x;
x=a[3];
}

int b[10][5];
int z()
{
int x;
x=b[3][5];
}

BB.f.0:
%T00001 = LEA a
%T00002 = MUL CONSTANT (INT): 3, CONSTANT (INT): 4
%T00003 = ADD %T00001, CONSTANT (INT): 3
%T00004 = LOAD %T00003
x = MOV %T00004

*/
