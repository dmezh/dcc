//!dtest description Postincrement.
//!dtest expect returncode 9

int main() {
    int i = 4;
    int j = i++; // 4

    return i + j; // 9
}
