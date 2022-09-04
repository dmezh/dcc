//!dtest description Ternary.
//!dtest expect returncode 99

int main() {
    int i = 11;
    int j = 22;

    int c = (i + j < 99) ? (i + j) : 99;

    return c ? c * 3 : 0;
}
