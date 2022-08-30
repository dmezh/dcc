//!dtest description Multi-dimensonal arrays.
//!dtest expect returncode 44

int main() {
    int i[10][5];
    i[5][1] = 33;
    i[1][0] = 11;
    return i[5][1] + i[1][0];
}
