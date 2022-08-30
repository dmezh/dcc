//!dtest description Direct dereference of simple arrays.
//!dtest expect returncode 90

int main() {
    int i[50];
    *i = 40;
    i[1] = 50;
    return *i + i[1];
}
