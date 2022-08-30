//!dtest description Direct dereference of multidimensional arrays.
//!dtest expect returncode 99

int main() {
    int i[10][20];
    *i[5] = 99;
    return *i[5];
}
