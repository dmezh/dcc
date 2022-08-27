//!dtest skip
//!dtest description "Test compilation of variadics, without testing accessing varargs themselves."

#include "../dcc_assert.h"

int a(int i, ...);

int a(int i, ...) {
    return i;
}

int main() {
    a(0);
    dcc_assert(a(1, 2, 3) == 1);
}
