#ifndef QUADS_CF_H
#define QUADS_CF_H

#include "quads.h"

struct cursor {
    char* fn;
    BB *brk, *cont;
};

void gen_if(astn* ifnode);
void gen_while(astn* wn);
void gen_dowhile(astn* dw);
void gen_for(astn* fl);
BBL* bbl_next(BBL* cur);
BB* bbl_data(BBL* n);
void bbl_append(BB* bb);
void uncond_branch(BB* b);
void gen_condexpr(astn *cond, BB* Bt, BB* Bf);
void emit_branch (int op, BB* Bt, BB* Bf);

extern struct cursor cursor;

#endif
