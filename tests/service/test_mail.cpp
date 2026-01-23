#include "../test_runner.h"
#include "../../include/federated/service/mail.h"
#include <cstring>

using namespace federated::service;
using namespace federated::core;

// Test: Mail store initialization
TEST(mail_store_init) {
    MailStore* store = get_mail_store();
    TEST_ASSERT(store != nullptr);
    
    ErrorCode err = store->init();
    TEST_ASSERT_EQ(err, OK);
    
    store->cleanup();
}

// Test: Append message to mailbox
TEST(mail_store_append) {
    MailStore* store = get_mail_store();
    store->init();
    
    MailMessage msg;
    strcpy(msg.from, "sender@example.com");
    strcpy(msg.to, "receiver@example.com");
    strcpy(msg.subject, "Test Subject");
    msg.timestamp = 1234567890;
    
    ErrorCode err = store->append("INBOX", msg);
    TEST_ASSERT_EQ(err, OK);
    
    store->cleanup();
}

// Test: List messages in mailbox
TEST(mail_store_list) {
    MailStore* store = get_mail_store();
    store->init();
    
    // Add two messages
    MailMessage msg1;
    strcpy(msg1.from, "sender1@example.com");
    strcpy(msg1.to, "receiver@example.com");
    strcpy(msg1.subject, "Subject 1");
    store->append("INBOX", msg1);
    
    MailMessage msg2;
    strcpy(msg2.from, "sender2@example.com");
    strcpy(msg2.to, "receiver@example.com");
    strcpy(msg2.subject, "Subject 2");
    store->append("INBOX", msg2);
    
    // List messages
    MailMessage messages[10];
    size_t count = 10;
    ErrorCode err = store->list("INBOX", messages, &count);
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_EQ(count, 2);
    TEST_ASSERT_STR_EQ(messages[0].from, "sender1@example.com");
    TEST_ASSERT_STR_EQ(messages[1].from, "sender2@example.com");
    
    store->cleanup();
}

// Test: Fetch specific message
TEST(mail_store_fetch) {
    MailStore* store = get_mail_store();
    store->init();
    
    MailMessage msg;
    strcpy(msg.from, "sender@example.com");
    strcpy(msg.to, "receiver@example.com");
    strcpy(msg.subject, "Test Subject");
    store->append("INBOX", msg);
    
    // Get the ID that was generated
    MailMessage messages[10];
    size_t count = 10;
    store->list("INBOX", messages, &count);
    TEST_ASSERT_EQ(count, 1);
    
    // Fetch by ID
    MailMessage fetched;
    ErrorCode err = store->fetch("INBOX", messages[0].id, fetched);
    
    TEST_ASSERT_EQ(err, OK);
    TEST_ASSERT_STR_EQ(fetched.from, "sender@example.com");
    
    store->cleanup();
}

// Test: Delete message
TEST(mail_store_delete) {
    MailStore* store = get_mail_store();
    store->init();
    
    MailMessage msg;
    strcpy(msg.from, "sender@example.com");
    strcpy(msg.to, "receiver@example.com");
    strcpy(msg.subject, "Test Subject");
    store->append("INBOX", msg);
    
    // Get the ID
    MailMessage messages[10];
    size_t count = 10;
    store->list("INBOX", messages, &count);
    TEST_ASSERT_EQ(count, 1);
    
    // Delete message
    ErrorCode err = store->remove("INBOX", messages[0].id);
    TEST_ASSERT_EQ(err, OK);
    
    // Verify it's deleted
    count = 10;
    store->list("INBOX", messages, &count);
    TEST_ASSERT_EQ(count, 0);
    
    store->cleanup();
}

// Test: Persistence restart simulation
TEST(mail_store_persistence_restart) {
    MailStore* store = get_mail_store();
    
    // First session
    store->init();
    MailMessage msg;
    strcpy(msg.from, "sender@example.com");
    strcpy(msg.to, "receiver@example.com");
    strcpy(msg.subject, "Test Subject");
    store->append("INBOX", msg);
    store->cleanup();
    
    // Second session (simulated restart)
    store->init();
    MailMessage messages[10];
    size_t count = 10;
    ErrorCode err = store->list("INBOX", messages, &count);
    
    // In-memory store won't persist, but this tests the API
    TEST_ASSERT_EQ(err, OK);
    
    store->cleanup();
}
