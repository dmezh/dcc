//!dtest description ptr<->integer comparisons.

#include "../dcc_assert.h"

int main() {
    int *i = 0;
    int *j = 0xFFFF;

    if (i)
      dcc_assert(0);

    if (j)
      dcc_assert(j);
    dcc_assert(j);

    if (i == 0)
      dcc_assert(i == 0);
    dcc_assert(i == 0);
    dcc_assert(j != 0);

    if (j != 0xFFFF)
      dcc_assert(0);
    dcc_assert(j == 0xFFFF);

    j = j ? 0 : 10;
    dcc_assert(j == 0);

    i = i ? i : 10;
    dcc_assert(i);
    dcc_assert(i == 10);
}
