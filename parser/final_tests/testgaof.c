int i;
int printf();
int main() {
	i=10;
	int *j; j=&i;
	printf("should be 10: %d\n", *j);
	return 0;
}

