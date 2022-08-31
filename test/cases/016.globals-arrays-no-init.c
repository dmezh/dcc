//!dtest description Global arrays, no initializers.
//!dtest expect returncode 99

int i[5];
int j[10];

int main() {
    i[1] = 33;
    j[8] = 66;
    return i[1] + j[8];
}
