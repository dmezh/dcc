int a(int i, ...);

int a(int i, ...) {
    return i;
}

int main() {
    a(0);
    return a(1, 2, 3);
}
