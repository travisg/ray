#pragma once

// Minimal test helpers: CHECK* report failures with file:line and count them;
// TEST_RESULT() is the process exit status.
#include <cmath>
#include <cstdio>

namespace test {
inline int failures = 0;
inline void Fail(const char *file, int line, const char *what) {
    std::fprintf(stderr, "%s:%d: check failed: %s\n", file, line, what);
    failures++;
}
} // namespace test

#define CHECK(expr)                                                                                \
    do {                                                                                           \
        if (!(expr)) {                                                                             \
            test::Fail(__FILE__, __LINE__, #expr);                                                 \
        }                                                                                          \
    } while (0)

#define CHECK_NEAR(a, b, eps)                                                                      \
    do {                                                                                           \
        if (!(std::fabs((a) - (b)) <= (eps))) {                                                    \
            test::Fail(__FILE__, __LINE__, #a " ~= " #b);                                          \
        }                                                                                          \
    } while (0)

#define TEST_RESULT() (test::failures == 0 ? 0 : 1)
