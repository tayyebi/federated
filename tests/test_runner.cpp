#include "test_runner.h"
#include <cstdio>

namespace federated {
namespace test {

TestStats g_test_stats;
static TestCase* g_test_head = nullptr;

void register_test(const char* name, TestFunc func) {
    TestCase* tc = new TestCase();
    tc->name = name;
    tc->func = func;
    tc->next = g_test_head;
    g_test_head = tc;
}

int run_all_tests() {
    printf("Running tests...\n");
    
    int test_count = 0;
    for (TestCase* tc = g_test_head; tc != nullptr; tc = tc->next) {
        printf("  [%s] ", tc->name);
        fflush(stdout);
        
        int passed_before = g_test_stats.passed;
        int failed_before = g_test_stats.failed;
        
        tc->func();
        
        int passed_now = g_test_stats.passed - passed_before;
        int failed_now = g_test_stats.failed - failed_before;
        
        if (failed_now == 0) {
            printf("OK (%d assertions)\n", passed_now);
        } else {
            printf("FAILED (%d/%d assertions failed)\n", failed_now, passed_now + failed_now);
        }
        
        test_count++;
    }
    
    printf("\n");
    printf("Summary: %d tests, %d assertions\n", test_count, g_test_stats.total);
    printf("  Passed: %d\n", g_test_stats.passed);
    printf("  Failed: %d\n", g_test_stats.failed);
    
    return g_test_stats.failed == 0 ? 0 : 1;
}

} // namespace test
} // namespace federated
