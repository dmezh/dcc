//!dtest description "For loops."

#include "../dcc_assert.h"

int main() {
    int ret = 0;

    int i_prev = -1;
    int ret_prev = ret;

    for (int i = 0; i < 3; i++) {
        dcc_assert(++i_prev == i);

        dcc_assert(ret_prev == ret);
        ret++;
        dcc_assert(ret_prev == ret - 1);
        ret_prev = ret;
        dcc_assert(ret_prev == ret);
    }

    dcc_assert(ret == 3);
}
