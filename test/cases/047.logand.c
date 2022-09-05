//!dtest description Logical and.

#include "../dcc_assert.h"

int main() {
    dcc_assert((0 && 0) == 0);
    dcc_assert((0 && 1) == 0);
    dcc_assert((1 && 0) == 0);
    dcc_assert((1 && 1) == 1);
}
