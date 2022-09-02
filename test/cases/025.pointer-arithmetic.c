//!dtest description Pointer subtraction.
//!dtest expect returncode 99

int main() {
        int i = 5;
        int j[10];
        *(&j[10] - i) = 66;

        *(&j[0] + i + 1) = 33;

        return j[5] + j[6];
}
