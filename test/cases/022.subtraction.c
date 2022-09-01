//!dtest description Test basic local vars.
//!dtest expect returncode 1

int main() {
    int i = 4;
    int j = 6 - i;
    j = j - 1;
    return j;
}
