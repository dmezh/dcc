int fib();
int fib(int x) {
        if (x == 0) return 0;
        if (x == 1) return 1;
        return fib(x-1) + fib(x-2);
}

// geeksforgeeks method 3
int fast_fib(int x) {
        int a; a=0;
        int b; b=1;
        int c; int i;

        if (x == 0) return a;

        i=2;
        while(i<=x) {
                c = a+b;
                a = b;
                b = c;
                i++;
        }
        return b;
}
