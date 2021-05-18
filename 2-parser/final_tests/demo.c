int printf();
int fprintf();
int putchar();
int getchar();
int strcpy();

extern void* stderr;

int q;
char bye[20];

int square(int i) {
	return i*i;
}

int dub(int i) {
	return i+i;
}

int main(int argc, char** argv) {
	int i;
	printf("dcc version 0\n~~~ Welcome to a demonstration of some of the abilities of dcc! ~~~\n");
	printf("please read along in the source for this file.\n");

	printf("\nFirst, let's print out the command-line arguments using a loop.\n");
	argc-=1;
	printf("Number of arguments: %d\n", argc);
	while (argc) {
		printf("Printing argument %d: \'%s\'\n", argc, argv[argc]);
		argc--;
	}
	putchar('\n');

	printf("Now let's print out the first 40 fibonacci numbers using the classic recursive routine.\n");
	printf("Ready when you are: press any key to continue. ");
	getchar();

	int newline; newline='\n';
	printf("%c", newline); // why not?!

	int fib(); // why not?!
	i=0;
	while (i < 40) {
		printf("%d ", fib(i++));
	}
	printf("\n\n");


	printf("Now let's print out the first 40 fibonacci numbers using a linear routine.\n");
	printf("Note that we'll use fprintf->stderr, so there won't be buffering.\n\n");
	printf("Ready when you are: press any key to continue. ");
	getchar(); putchar('\n');

	int fast_fib(int x); // also ok!
	i=0;
	while (i < 40) {
		fprintf(stderr, "%d ", fast_fib(i++));
	}
	printf("\n\n");

	printf("Now let's do some pointer juggling\n");
	int x;
	int *y; y=&x;
	int **z; z=&y;
	int ***zz; zz=&z;

	***zz = 2021;
	printf("You should see 2021 here: %d\n", x);

	y = &q;
	*y = 8086;
	printf("Let's do the same for a global: %d\n", q);

	printf("A little less fancy: %d\n\n", ++q);

	printf("Let's try arrays!\n");
	int aa[10];
	printf("Going to write a sequence into array aa[], counting backwards from 68000\n");
	i=68000;
	while ((68000-i)<10) {
		aa[68000-i] = i;
		i--;
	}

	printf("And now to dump the array:\n");
	i=0;
	while (i < (sizeof(aa)/sizeof(int)))   // sure!
		printf("aa[%d]=%d\n", i++, aa[i]); // NOTICE: weird behavior! we evaluated the arguments
										   // right to left, as we are x86_32.

	printf("Let's try some conditional stuff.\n");

	if (aa[1] > 1000) {
		printf("aa[1] is pretty big!\n");

		if (aa[1] >= 67999) {
			printf("aa[1] is REALLY big!\n");

			if (aa[1] >= 68000)
				printf("aa[1] is the hugest!\n");
			else
				printf("but not the absolute biggest.\n");

		} else {
			printf("aa[1] not THAT big though.\n");
		}

	} else {
		printf("aa[1] isn't really big at all.\n");
	}

	printf("\nLet's call a function in the same file, for kicks.\n");
	printf("The double(2*) of the square of 3 is: %d\n\n", dub(square(3)));

	printf("Finally, let's do some math.\n");
	int a; a=21;
	int b; b=4;
	printf("const int div: 21/4 should be 5: %d\n", 21/4);
	printf("const int mod: 21%4 should be 1: %d\n", 21%4);
	printf("var div: a/b==21/4 should be 5: %d\n", a/b);
	printf("var mod: a%b==21%4 should be 1: %d\n", a%b);
	printf("a is %d, b is %d\n", a, b);
	printf("(a>b) evaluates to:%d\n", (a>b));
	printf("(a<b) evaluates to:%d\n", (a<b));
	printf("(a==b) evaluates to:%d\n", (a==b));
	printf("(a!=b) evaluates to:%d\n", (a!=b));

	printf("\nglobal arrays work too, even char[] if you're careful...\n");
	strcpy(bye, "thanks and cya!");
	bye[15] = '\n';
	bye[16] = '\0'; // adding a newline and null-terminating
	printf(bye);

	return 0;
}

