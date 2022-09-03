//!dtest description Unary ++ preincrement.
//!dtest expect returncode 6

int main() {
    int i = 0;
    return ++i + ++i + ++i;
}
