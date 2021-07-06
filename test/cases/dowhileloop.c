int main() {
    int i;
    i = 2;

    do {
        i+=1;
    } while (i < 0);

    do {
        i*=i;
    } while (i<50);

    return i;
}
