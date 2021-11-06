#include "opt_flatten_adjacent_mov.h"

#include <stdlib.h>

#include "quads.h"
#include "quads_cf.h"

void try_replace_temp(astn* n, struct astn_qtemp* del, struct astn_qtemp* replace) {
    if (n && n->type == ASTN_QTEMP && n->astn_qtemp.tempno == del->tempno)
        n->astn_qtemp = *replace;
}

void opt_flatten_adjacent_mov(BBL* root, unsigned passes) {
    BBL* head = root;
    while (passes--) {
        BBL* bbl = head;
        if (head == &bb_root) bbl = bbl_next(bbl);

        while (bbl) {
            BB* b = bbl_data(bbl);
            bbl = bbl_next(bbl);
            quad *q = b->start;

            while (q && q->next && q->next->next) {
                quad *n = q->next;
                quad *nn = n->next;
                if ((q->op == Q_MOV && n->op == Q_MOV) &&
                    (q->target->type == ASTN_SYMPTR && q->src1->type == ASTN_QTEMP) &&
                    (n->target->type == ASTN_QTEMP && n->src1->type == ASTN_SYMPTR)){
                    //(q->src1->astn_symptr.e == n->target->astn_symptr.e)) { // this line broken
                        fprintf(stderr, "found!\n");
                        // we found the pattern.
                        // now we will remove the second quad entirely
                        // and replace all references to its target with the parent's source.
                        struct astn_qtemp* del = &n->target->astn_qtemp;
                        struct astn_qtemp* replace = &q->src1->astn_qtemp;
                        fprintf(stderr, "del is %u, replace is %u\n", del->tempno, replace->tempno);
                        
                        q->next = nn;
                        free(n); // todo: potentially still leaky?
                        
                        while (nn) {
                            try_replace_temp(nn->target, del, replace);
                            try_replace_temp(nn->src1, del, replace);
                            try_replace_temp(nn->src2, del, replace);
                            nn = nn->next;
                        }
                }
                q = q->next;
            }
        }
    }
}

