#ifndef DCC_ASSERT_T
#define DCC_ASSERT_H

void __assert_fail(const char *expr, const char *file, int line, const char *func);

#define dcc_assert(expr) if (!(expr)) __assert_fail(#expr, __FILE__, __LINE__, "{unknown}")

#endif
