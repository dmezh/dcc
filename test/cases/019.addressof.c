//!dtest description Addressof
//!dtest expect returncode 99

int main() {
    int i = 0;
    int *ii = &i;

    *ii = 99;

    return *ii;
}
