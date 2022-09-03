//!dtest description _Bool type.
//!dtest expect returncode 2

#include <stdbool.h>

int main() {
    bool c = 7; // should be 1
    return c + c; // promotions -> (int)1 + (int)1 = 2
}


