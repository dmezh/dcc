int i[10];
int printf();
int main() {
	i[3] = 5;
	i[0] = 1;
	i[8] = 100;
	i[1] = 100;
	i[9] = 1;
	printf("should be 7: %d\n", i[3]+i[0]+i[9]);
	return 0;
}
