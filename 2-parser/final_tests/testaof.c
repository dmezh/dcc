int printf();

int main() {
	int i; i=10;
	int *j; j=&i;
	printf("%d\n", (*j));
	return 0;
}
