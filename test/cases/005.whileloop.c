//!dtest description "While loops."

#include "../dcc_assert.h"

int printf();

int main() {
    int i=2;

    dcc_assert(i == 2);

    while (i<50) {
        i*=2;
    }

    dcc_assert(i == 64);
}
