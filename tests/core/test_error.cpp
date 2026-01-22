#include "../test_runner.h"
#include "../../include/federated/core/error.h"
#include <cstring>

using namespace federated::core;

TEST(error_codes_defined) {
    TEST_ASSERT_EQ(OK, 0);
    TEST_ASSERT_NE(ERR_IO, 0);
    TEST_ASSERT_NE(ERR_FORMAT, 0);
    TEST_ASSERT_NE(ERR_TIMEOUT, 0);
    TEST_ASSERT_NE(ERR_CRYPTO, 0);
    TEST_ASSERT_NE(ERR_UNSUPPORTED, 0);
    TEST_ASSERT_NE(ERR_AUTH, 0);
    TEST_ASSERT_NE(ERR_INTERNAL, 0);
}

TEST(error_codes_unique) {
    TEST_ASSERT_NE(ERR_IO, ERR_FORMAT);
    TEST_ASSERT_NE(ERR_IO, ERR_TIMEOUT);
    TEST_ASSERT_NE(ERR_FORMAT, ERR_CRYPTO);
}

TEST(error_message_ok) {
    const char* msg = error_message(OK);
    TEST_ASSERT(msg != nullptr);
    TEST_ASSERT(strlen(msg) > 0);
}

TEST(error_message_io) {
    const char* msg = error_message(ERR_IO);
    TEST_ASSERT(msg != nullptr);
    TEST_ASSERT(strlen(msg) > 0);
}
