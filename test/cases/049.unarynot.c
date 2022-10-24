//!dtest description Unary !.

#include "../dcc_assert.h"

int main() {
    int *i = 0;
    int j = 0;
    int k = 99;

    dcc_assert(!i);
    dcc_assert(!j);
    dcc_assert(!!!i);
    dcc_assert(!!!j);
    dcc_assert(k);
    dcc_assert(!!k);

    dcc_assert(!!i == 0);

    i = 0xFF;

    if (!i)
        dcc_assert(0);

    i = 0;
    
    if (!!i)
        dcc_assert(0);
    
    if (!!i != !k)
        dcc_assert(0);
}
