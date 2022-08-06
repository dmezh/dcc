int printf();

int main() {
	int a; a=21;
	int b; b=4;
	printf("const int div: 21/4 should be 5: %d\n", 21/4);
	printf("const int mod: 21%4 should be 1: %d\n", 21%4);
        printf("var div: a/b==21/4 should be 5: %d\n", a/b);
        printf("var mod: a%b==21%4 should be 1: %d\n", a%b);
	return 0;
}
