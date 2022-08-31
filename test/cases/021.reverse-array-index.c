//!dtest description Funny form of array indexing.
//!dtest expect returncode 99

int main() {
    int i[10];

    5[i] = 99;

    return 5[i];
}
