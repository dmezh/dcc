//!dtest description "Do-while loops."

#include "../dcc_assert.h"

int main() {
    int i = 2;

    dcc_assert(i == 2);

    do {
        i+=1;
    } while (i < 0);

    dcc_assert(i == 3);

    do {
        i*=i;
    } while (i<50);

    dcc_assert(i == 81);

    return 0;
}
