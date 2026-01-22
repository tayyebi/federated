#ifndef FEDERATED_TEST_RUNNER_H
#define FEDERATED_TEST_RUNNER_H

#include <cstdio>
#include <cstring>

namespace federated {
namespace test {

// Simple test framework without external dependencies
struct TestStats {
    int total;
    int passed;
    int failed;
    
    TestStats() : total(0), passed(0), failed(0) {}
};

extern TestStats g_test_stats;

// Test assertion macros
#define TEST_ASSERT(condition) \
    do { \
        federated::test::g_test_stats.total++; \
        if (condition) { \
            federated::test::g_test_stats.passed++; \
        } else { \
            federated::test::g_test_stats.failed++; \
            fprintf(stderr, "FAIL: %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        } \
    } while(0)

#define TEST_ASSERT_EQ(a, b) \
    do { \
        federated::test::g_test_stats.total++; \
        if ((a) == (b)) { \
            federated::test::g_test_stats.passed++; \
        } else { \
            federated::test::g_test_stats.failed++; \
            fprintf(stderr, "FAIL: %s:%d: %s == %s\n", __FILE__, __LINE__, #a, #b); \
        } \
    } while(0)

#define TEST_ASSERT_NE(a, b) \
    do { \
        federated::test::g_test_stats.total++; \
        if ((a) != (b)) { \
            federated::test::g_test_stats.passed++; \
        } else { \
            federated::test::g_test_stats.failed++; \
            fprintf(stderr, "FAIL: %s:%d: %s != %s\n", __FILE__, __LINE__, #a, #b); \
        } \
    } while(0)

#define TEST_ASSERT_STR_EQ(a, b) \
    do { \
        federated::test::g_test_stats.total++; \
        if (strcmp(a, b) == 0) { \
            federated::test::g_test_stats.passed++; \
        } else { \
            federated::test::g_test_stats.failed++; \
            fprintf(stderr, "FAIL: %s:%d: strcmp(%s, %s) failed\n", __FILE__, __LINE__, #a, #b); \
        } \
    } while(0)

// Test function registration
typedef void (*TestFunc)();

struct TestCase {
    const char* name;
    TestFunc func;
    TestCase* next;
};

void register_test(const char* name, TestFunc func);
int run_all_tests();

// Test registration macro
#define TEST(name) \
    void test_##name(); \
    namespace { \
        struct TestRegistrar_##name { \
            TestRegistrar_##name() { \
                federated::test::register_test(#name, test_##name); \
            } \
        }; \
        static TestRegistrar_##name g_test_registrar_##name; \
    } \
    void test_##name()

} // namespace test
} // namespace federated

#endif // FEDERATED_TEST_RUNNER_H
