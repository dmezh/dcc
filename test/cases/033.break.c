//!dtest description Cursor break.
//!dtest expect returncode 1

int main() {
    int i = 0;
    while (i != 10) {
            i++;
            break;
    }

    return i;
}
