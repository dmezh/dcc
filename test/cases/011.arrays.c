//!dtest description Simple arrays.
//!dtest expect returncode 105

int main() {
    int i[10];
    i[6] = 105;
    return i[6];
}
