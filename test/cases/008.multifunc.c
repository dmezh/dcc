//!dtest description "Multiple functions."
//!dtest expect returncode 98

int add(int a, int b);
int multiply_by_adding(int a, int b);
int indirect_add(int a, int b);

int main() {
    int a = 2;
    int b = 6;

    int c;
    c = add(a, b);

    c = multiply_by_adding(c, a);

    c = multiply_by_adding(c, b);

    c = indirect_add(c, a);

    return c;
}

int add(int a, int b) {
    return a+b;
}

int multiply_by_adding(int a, int b) {
    int ret = 0;
    for (int i = 0; i < a; i++) {
        ret += b;
    }
    return ret;
}

int indirect_add(int a, int b) {
    return add(a, b);
}
