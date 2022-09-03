//!dtest description Arrays of structs.
//!dtest expect returncode 10

struct s {
    int i;
    int j;
    char c;
};

int main() {
    struct s s[10];
    s[5].i = 1;
    s[3].j = 9;
    return s[5].i + s[3].j;
}
