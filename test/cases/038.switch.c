//!dtest description switch/case.
//!dtest expect returncode 40

int main() {
    int i = 2;
    int z;

    switch (i) {
        case 0:
            z = 0;
        case 1:
            z = 14;
            break;
        case 2:
            z = 26;
        case 3:
            z = 38;
            break;
        default:
        {
            break;
        }
    }

    return z + i;
}
