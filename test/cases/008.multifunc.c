//!dtest description "Multiple functions."

#include "../dcc_assert.h"

int add();
int multiply();
int multiply_by_adding();
int indirect_add();

int main() {
    int a = 2;
    int b = 6;

    int c;
    c = add(a, b);
    dcc_assert(c == 8);

    c = multiply(c, a);
    dcc_assert(c == 16);

    c = multiply_by_adding(c, b);
    dcc_assert(c == 96);

    c = indirect_add(c, a);
    dcc_assert(98);
}

int add(int a, int b) {
    return a+b;
}

int multiply(int a, int b) {
    return a * b;
}

int multiply_by_adding(int a, int b) {
    int ret = 0;
    for (int i = 0; i < a; i++) {
        ret += b;
    }
    return ret;
}

int indirect_add(int a, int b) {
    return add(a, b);
}
