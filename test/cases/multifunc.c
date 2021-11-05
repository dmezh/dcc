int add();
int multiply();
int multiply_by_adding();
int indirect_add();

int main() {
    int a; int b;
    a = 2; b = 6;

    int c;
    c = add(a, b);
    c = multiply(c, a);
    c = multiply_by_adding(c, b);
    c = indirect_add(c, a);

    return c;
}

int add(int a, int b) {
    return a+b;
}

int multiply(int a, int b) {
    return a * b;
}

int multiply_by_adding(int a, int b) {
    int i;
    int ret;
    ret = 0;
    for (i = 0; i < a; i++) {
        ret += b;
    }
    return ret;
}

int indirect_add(int a, int b) {
    return add(a, b);
}
