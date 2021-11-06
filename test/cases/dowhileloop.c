int main() {
    int i = 2;

    do {
        i+=1;
    } while (i < 0);

    do {
        i*=i;
    } while (i<50);

    return i;
}
