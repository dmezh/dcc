//!dtest description Test pointer casting and underlying type correctness.
//!dtest expect returncode 66

// note: counting elements from 0 - i[0][0] is 0, i[1][0] is 11
int main() {
    int i[5][10];

    i[1][1] = 33; // element 11
    int (*ii)[5] = &i;


    int (*cc)[10]; // cc[1] should be element 10
    cc = ii;

    // ii[2][1] is element 11
    return cc[1][1] + ii[2][1];
}
