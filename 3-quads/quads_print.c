#include "quads_print.h"

#include "ast.h"
#include "quads.h"
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
    [Q_LEA] = "LEA"
};

void print_node(const astn* qn) {
    switch (qn->type) {
        case ASTN_SYMPTR:   printf("%s", qn->astn_symptr.e->ident); return;
        case ASTN_NUM:      print_number(&qn->astn_num.number); return;
        case ASTN_QTEMP:    printf("%%T%05d", qn->astn_qtemp.tempno); return;
        default: die("eh");
    }
}

void print_quad(quad* q) {
    if (q->target) {
        print_node(q->target);
        printf(" = %s ", quad_op_str[q->op]);
    }
    if (q->src1) {
        print_node(q->src1);
    }
    if (q->src2) {
        printf(", ");
        print_node(q->src2);
    }
    printf("\n");
}
