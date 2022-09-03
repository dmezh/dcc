//!dtest description While loops.
//!dtest expect returncode 64

int main() {
    int i=2;

    while (i != 64) {
        i+=i;
    }

    return i;
}
