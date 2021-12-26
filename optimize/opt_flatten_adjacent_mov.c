#include "opt_flatten_adjacent_mov.h"

#include <stdlib.h>

#include "quads.h"
#include "quads_cf.h"

unsigned total_replaced = 0;

void try_replace_temp(astn* n, struct astn_qtemp* del, struct astn_qtemp* replace) {
    if (n && n->type == ASTN_QTEMP && n->Qtemp.tempno == del->tempno)
        n->Qtemp = *replace;

    total_replaced++;
}

void opt_flatten_adjacent_mov(BBL* root, unsigned passes) {
    BBL* head = root;
    unsigned hits = 0;
    for (unsigned pass = 1; pass <= passes; pass++) {
        // fprintf(stderr, "Pass %d/%d running...\n", pass, passes);
        unsigned this_hits = 0, quads_visited = 0;
        BBL* bbl = head;
        if (head == &bb_root) bbl = bbl_next(bbl);

        while (bbl) {
            BB* b = bbl_data(bbl);
            bbl = bbl_next(bbl);
            quad *q = b->start;

            while (q && q->next && q->next->next) {
                quads_visited++;
                quad *n = q->next;
                quad *nn = n->next;
                if ((q->op == Q_MOV && n->op == Q_MOV) &&
                    (q->target->type == ASTN_SYMPTR && q->src1->type == ASTN_QTEMP) &&
                    (n->target->type == ASTN_QTEMP && n->src1->type == ASTN_SYMPTR) &&
                    (q->target->Symptr.e == n->src1->Symptr.e)) {
                        this_hits++;
                        // we found the pattern.
                        // now we will remove the second quad entirely
                        // and replace all references to its target with the parent's source.
                        struct astn_qtemp* del = &n->target->Qtemp;
                        struct astn_qtemp* replace = &q->src1->Qtemp;
                        //fprintf(stderr, "del is %u, replace is %u\n", del->tempno, replace->tempno);
                        
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
        hits += this_hits;
        // fprintf(stderr, "--> Quads visited: %d | hits for this pass: %d, all passes: %d\n", quads_visited, this_hits, hits);
    }
    BBL* bbl = head;
    if (head == &bb_root) bbl = bbl_next(bbl);


    unsigned remaining_quads = 0;
    while (bbl) {
        BB* b = bbl_data(bbl);
        bbl = bbl_next(bbl);
        quad *q = b->start;
        while (q) {
            q = q->next;
            remaining_quads++;
        }
    }
    // fprintf(stderr, "Remaining quads after optimization: %d\n", remaining_quads);
}
