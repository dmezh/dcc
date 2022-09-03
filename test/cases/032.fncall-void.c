//!dtest description Call void function.
//!dtest expect returncode 99

void putchar();

int main() {
    int i = 99;
    putchar('h');
    putchar('e');
    putchar('l');
    putchar('l');
    putchar('o');
    putchar('\n');
    return i;
}
