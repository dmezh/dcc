//!dtest description Test basic local vars.
//!dtest expect returncode 10

int main() {
    int i = 4;
    int j = 6 + i;
    return j;
}
