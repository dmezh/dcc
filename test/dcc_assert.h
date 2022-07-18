#ifndef DCC_ASSERT_H
#define DCC_ASSERT_H

// #define stderr ___stderrp

static void __assert_dcc(const char * assertion, const char * file, unsigned int line, const char * function)
{
    int printf();
    // extern void *___stderrp;
    printf("Assertion failed at %s::%s:%d - failed \"%s\"\n", file, function, line, assertion);

    void exit();
    exit(-5);
}

#define dcc_assert(expr) if (!(expr)) __assert_dcc(#expr, __FILE__, __LINE__, "{unknown}")

#endif
