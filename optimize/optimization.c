#include "optimization.h"

#include "opt_flatten_adjacent_mov.h"

void dcc_optimize(BBL* head) {
    opt_flatten_adjacent_mov(head, 1);
}
