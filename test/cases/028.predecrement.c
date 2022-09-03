//!dtest description Unary -- predecrement.
//!dtest expect returncode 24

int main() {
    int i = 10;
    return --i + --i + --i;
}
