//!dtest description Struct access.
//!dtest expect returncode 33

struct nb {
    struct c {
        int c;
        char i;
    } w;

    long z;
};

int main() {
    struct nb x;

    x.w.c = 0;
    x.w.i = 542;

    x.z = x.w.i + 3;

    return x.z;
}
