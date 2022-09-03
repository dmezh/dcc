//!dtest description Odd pointer chains; can be hard to get right.
//!dtest expect returncode 100

int main()
{
    int x = 100;
    int *p = &x - 1;
    int **pp = &p - 1;
    int ***ppp = &pp - 1;
    return *++*++*++ppp;
}
