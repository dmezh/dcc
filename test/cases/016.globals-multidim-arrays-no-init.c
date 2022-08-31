//!dtest description Multidimensional global arrays, no initializers.
//!dtest expect returncode 99

int i[5][10];

int main() {
    i[4][9] = 99;
    return i[4][9];
}
