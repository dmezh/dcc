#ifndef QUADS_CF_H
#define QUADS_CF_H

#include "quads.h"

struct cursor {
    char* fn;
};

void gen_if(astn* ifnode);
BBL* bbl_next(BBL* cur);
BB* bbl_data(BBL* n);
void bbl_append(BB* bb);
void uncond_branch(BB* b);
void gen_condexpr(astn *cond, BB* Bt, BB* Bf);
void emit_branch (int op, BB* Bt, BB* Bf);

extern struct cursor cursor;

#endif
