//!dtest description Local static variables.
//!dtest expect returncode 39

int f(int a) {
    static int i = 0;

    i += a;

    return i;
}

int main() {
    static int i = 0;

    i += f(10); // +10
    i += f(19); // +29

    return i;
}
