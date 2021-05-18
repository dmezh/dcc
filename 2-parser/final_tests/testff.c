int square(int i) {
	return i*i;
}

int dub(int i) {
	return i+i;
}

int printf();
int main() {
	printf("dub of square of 3 is %d\n", dub(square(3)));
	return 0;
}
