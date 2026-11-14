/* SPDX-License-Identifier: MIT */
/**
 * @file bs_test.h
 * @brief A unit-test harness small enough to read in one sitting.
 *
 * Each test file is one executable. It declares its cases with TEST(), lists
 * them in a table and ends with BS_TEST_MAIN(). Output follows the Test
 * Anything Protocol, and the process exits non-zero if any check failed.
 *
 *     TEST(adds_numbers) { CHECK_EQ(4, 2 + 2); }
 *
 *     static const bs_test_case TESTS[] = { BS_CASE(adds_numbers) };
 *     BS_TEST_MAIN(TESTS)
 *
 * Running `./test_x name` executes only the cases whose name contains "name".
 */
#ifndef BS_TEST_H
#define BS_TEST_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct bs_test_case {
    const char *name;
    void (*run)(void);
} bs_test_case;

/* Failed checks of the case being run. One translation unit per executable.
 * The helpers are inline so that a file using only some of them stays warning-free. */
static int bs_test_failures_;

static inline void bs_test_fail_(const char *file, int line, const char *message)
{
    fprintf(stderr, "# %s:%d: check failed: %s\n", file, line, message);
    bs_test_failures_++;
}

static inline void bs_test_fail_eq_(const char *file, int line, const char *expected_text,
                                    const char *actual_text, long long expected, long long actual)
{
    fprintf(stderr, "# %s:%d: expected %s == %s, got %lld vs %lld\n", file, line, expected_text,
            actual_text, expected, actual);
    bs_test_failures_++;
}

static inline int bs_test_run_(const bs_test_case *cases, size_t count, const char *filter)
{
    int failed_cases = 0;
    int executed = 0;

    for (size_t i = 0; i < count; i++) {
        if (filter != NULL && strstr(cases[i].name, filter) == NULL) {
            continue;
        }
        bs_test_failures_ = 0;
        cases[i].run();
        executed++;
        if (bs_test_failures_ == 0) {
            printf("ok %d - %s\n", executed, cases[i].name);
        } else {
            printf("not ok %d - %s\n", executed, cases[i].name);
            failed_cases++;
        }
    }
    printf("1..%d\n", executed);
    if (failed_cases > 0) {
        fprintf(stderr, "# %d of %d test case(s) failed\n", failed_cases, executed);
    }
    return failed_cases == 0 ? 0 : 1;
}

/** Defines a test case. */
#define TEST(name) static void name(void)

/** Table entry for a test case. */
#define BS_CASE(name) {#name, name}

/** Records a failure when the condition is false; the case keeps running. */
#define CHECK(condition)                                                                           \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            bs_test_fail_(__FILE__, __LINE__, #condition);                                         \
        }                                                                                          \
    } while (0)

/** Like CHECK(), but leaves the test case at once. For preconditions. */
#define REQUIRE(condition)                                                                         \
    do {                                                                                           \
        if (!(condition)) {                                                                        \
            bs_test_fail_(__FILE__, __LINE__, #condition);                                         \
            return;                                                                                \
        }                                                                                          \
    } while (0)

/** Compares two integer expressions and prints both values on mismatch. */
#define CHECK_EQ(expected, actual)                                                                 \
    do {                                                                                           \
        const long long bs_expected_ = (long long)(expected);                                      \
        const long long bs_actual_ = (long long)(actual);                                          \
        if (bs_expected_ != bs_actual_) {                                                          \
            bs_test_fail_eq_(__FILE__, __LINE__, #expected, #actual, bs_expected_, bs_actual_);    \
        }                                                                                          \
    } while (0)

/** Emits main() for the given table of cases. */
#define BS_TEST_MAIN(table)                                                                        \
    int main(int argc, char **argv)                                                                \
    {                                                                                              \
        return bs_test_run_(table, sizeof(table) / sizeof((table)[0]), argc > 1 ? argv[1] : NULL); \
    }

#endif /* BS_TEST_H */
