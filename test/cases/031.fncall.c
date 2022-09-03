//!dtest description Basic function calling.
//!dtest expect returncode 30

int putchar(int c);

int main() {
    return putchar('h') + putchar('e') + putchar('l') + putchar('l') + putchar('o') + putchar('\n');
}
