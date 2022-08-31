//!dtest description Assignment itself is an rvalue.
//!dtest expect returncode 99

int main() {
    int i = 5;
    return i = 99;
}
