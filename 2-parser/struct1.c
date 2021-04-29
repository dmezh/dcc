
static struct str1 {
	int a;
	/*const int i;	/*What happens when you uncomment this?*/
	struct str2 {
		short d;
		char c;
	} str2;
	char c;
} s1;

struct str2 s2;

struct str3 {
	struct str4 *p4;
} s3;

struct str4 {
	struct str3 *p3;
	struct str1 str1[10];
	int i;
} s4;

struct str6 {
	int a;
	unsigned b;
	unsigned c;
	unsigned d;
	unsigned e;
} s6;
