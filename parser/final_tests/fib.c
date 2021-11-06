int printf();
static int z; // just for seeing the .local
int fib(int i);

int main() {
	int i; i=0;
	while (i < 10) {
		printf("%d ", fib(i));
		i++;
	}
	printf("\n");
}

int fib(int x) {
	if (x == 0) return 0;
	if (x == 1) return 1;
	return fib(x-1) + fib(x-2);
}
