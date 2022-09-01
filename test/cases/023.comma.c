//!dtest description Comma operator.
//!dtest expect returncode 4

int main() {
    int i = 0;
    return i++, i + 100, i + 3;
}
