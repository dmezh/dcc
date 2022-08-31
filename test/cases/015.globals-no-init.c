//!dtest description Simple globals, no initializers.
//!dtest expect returncode 99

int i;

int main() {
    i = 99;
    return i;
}
