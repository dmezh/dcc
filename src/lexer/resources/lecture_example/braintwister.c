
main()
{
	f();
}


int a=1000;

int f(void)
{
	int *p;
	{
	  int a=1; // block scope a (BSa)
	  	p= &a; // pointer to BSa
		x: printf("%d\n",a); // print BSa
		a++; // increment BSa
	}
	if (*p<5) goto x; // if BSa is less than 5
	printf("%d\n",a); // print global a (1000)
}
	
