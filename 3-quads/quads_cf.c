#include "quads_cf.h"

#include "ast.h"
#include "parser.tab.h"
#include "quads.h"
#include "quads_print.h"
#include "util.h"

#include <stdio.h>

BB root_bb = {0}; // should have no pred, no bb, only a succ
BB* current_bb = &root_bb;
unsigned bb_count = 0;

BBL bb_root = {0};

struct cursor cursor = {0};

BB* bb_alloc() {
    BB *n = safe_calloc(1, sizeof(BB));
    n->bbno = bb_count++;
    //n->pred = current_bb;
    n->fn = cursor.fn;
    //printf("n is %p\n", (void*)n);
    //printf("me append%p\n", bbl_append(n)); // slight crime with the cast; should be ok
    bbl_append(n);
    //printf("n from list is %p\n", bbl_data(n));
    //printf("made new BB #%d\n", n->bbno);
    return n;
}

BBL* bbl_next(BBL* cur) {
    return cur->next;
}

BB* bbl_data(BBL* n) {
    return n->me;
}

void bbl_append(BB* bb) {
    BBL *new = safe_calloc(1, sizeof(BBL));
    new->me = bb;

    BBL *head = &bb_root;
    while (head->next) head=head->next;

    head->next = new;
    return;
}


static astn* wrap_bb(BB* bb) {
    astn *n = astn_alloc(ASTN_QBBNO);
    n->astn_qbbno.bb = bb;
    return n;
}

void gen_condexpr(astn *cond, BB* Bt, BB* Bf) {
    //printf("gen_condexpr asked to generate this condition:\n"); print_ast(cond);
    //printf("generating cond expr, my current BB number is %d\n", current_bb->bbno);
    switch (cond->type) {
        case ASTN_BINOP:
            ;
            astn *left = gen_rvalue(cond->astn_binop.left, NULL);
            astn *right = gen_rvalue(cond->astn_binop.right, NULL);
            /*
            printf("binop found, left and right prepared. Dumping nodes:\nL:\n");
            print_node(left);
            printf("\nR:\n");
            print_node(right);
            printf("\n");
            */

            emit(Q_CMP, left, right, NULL);
            emit_branch(cond->astn_binop.op, Bt, Bf);

            break;

        default:
            die("non-binop condexpr");
    }
}

void emit_branch (int op, BB* Bt, BB* Bf) {
    switch (op) {
                case '<':
                    //printf("detected LT\n");
                    emit(Q_BRGE, wrap_bb(Bf), wrap_bb(Bt), NULL);
                    break;
                case '>':
                    emit(Q_BRLE, wrap_bb(Bf), wrap_bb(Bt), NULL);
                    break;
                case LTEQ:
                    emit(Q_BRGT, wrap_bb(Bf), wrap_bb(Bt), NULL);
                    break;
                case GTEQ:
                    emit(Q_BRLT, wrap_bb(Bf), wrap_bb(Bt), NULL);
                    break;
                case EQEQ:
                    emit(Q_BRNE, wrap_bb(Bf), wrap_bb(Bt), NULL);
                    break;
                case NOTEQ:
                    emit(Q_BREQ, wrap_bb(Bf), wrap_bb(Bt), NULL);
                    break;
                default:
                    die("unhandled condexpr");
            }
}

void uncond_branch(BB* b) {
    emit(Q_BR, wrap_bb(b), NULL, NULL);
}

void gen_if(astn* ifnode) {
    struct astn_ifelse *if_node = &ifnode->astn_ifelse;
    BB* Bt = bb_alloc();
    BB* Bf = bb_alloc();
    BB* Bn;

    if (if_node->else_s)
        Bn = bb_alloc();
    else
        Bn = Bf;

    gen_condexpr(if_node->condition_s, Bt, Bf);
    current_bb = Bt;
    gen_quads(if_node->then_s);
    uncond_branch(Bn);

    if (if_node->else_s) {
        current_bb = Bf;
        gen_quads(if_node->else_s);
        uncond_branch(Bn);
    }

    current_bb = Bn;
}

void gen_while(astn* wn) {
    struct astn_whileloop *w = &wn->astn_whileloop;
    BB* cond = bb_alloc();
    BB* body = bb_alloc();
    BB* next = bb_alloc();

    uncond_branch(cond);

    current_bb = cond;
    gen_condexpr(w->condition, body, next);

    current_bb = body;
    cursor.brk = next;
    cursor.cont = cond;
    gen_quads(w->body);
    uncond_branch(cond);

    current_bb = next;
}

/*
BB.y.0:
%T00005 = MOV a
CMP %T00005, CONSTANT (INT): 3
BRGE BB.y.2, BB.y.1

BB.y.1:
a = MOV CONSTANT (INT): 1
BR BB.y.3

BB.y.2:
%T00006 = MOV a
CMP %T00006, CONSTANT (INT): 2
BRGE BB.y.5, BB.y.4

BB.y.3:
%T00007 = MOV a
%T00008 = ADD %T00007, CONSTANT (INT): 1
a = MOV %T00008

BB.y.4:
a = MOV CONSTANT (INT): 2
BR BB.y.6

BB.y.5:
a = MOV CONSTANT (INT): 3
BR BB.y.6

BB.y.6:
BR BB.y.3

int a; int b; int c; int d; int e; int f; int g;
int y() {
if (a<3) a=1;
else if (a<2) a=2;
else a=3;

a=a+1;
}


*/
