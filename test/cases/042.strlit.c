//!dtest description String literals.
//!dtest expect returncode 8

int strlen(const char *s);

int main() {
    return strlen("hello!\n\n");
}
