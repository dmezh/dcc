//!dtest description Nested ternaries
//!dtest expect returncode 99

int main() {
    return (1 > 100)
        ? ((5 > 10) ? (100 * 3) : (200 * 2))
        : ((2 - 2) ? (6 + 8) : 99);
}
