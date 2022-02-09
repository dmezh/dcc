#pragma do-dumpsymtab
int i;
#pragma do-dumpsymtab

int main() {
    #pragma do-dumpsymtab
    int j;
    #pragma do-dumpsymtab
    #pragma do-examine i
    return 0;
}

#pragma do-examine i

