//!dtest description Ternary with struct members.
//!dtest expect returncode 99

int main() {
    struct s { int i; int j; int k; } s;

    s.i = 3;
    s.k = 33;

    *(s.k > 10 ? &s.i : &s.j) = 99;

    return s.i;
}
