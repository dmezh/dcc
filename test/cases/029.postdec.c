//!dtest description Postdecrement.
//!dtest expect returncode 7

int main() {
    int i = 4;
    int j = i--; // 4

    return i + j; // 7
}
