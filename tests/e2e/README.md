# End-to-End (E2E) Tests

This directory contains end-to-end tests for Federated services. These tests validate complete workflows and service interactions.

## Test Structure

- `lib.sh` - Test framework and utilities
- `run_all.sh` - Master test runner
- `test_smtp.sh` - SMTP service tests
- `test_imap.sh` - IMAP service tests
- `test_combined.sh` - Combined service tests

## Running Tests

### Run All Tests

```bash
./tests/e2e/run_all.sh
```

### Run Individual Test Suite

```bash
./tests/e2e/test_smtp.sh
./tests/e2e/test_imap.sh
./tests/e2e/test_combined.sh
```

### Custom Binary Path

```bash
FEDERATED_BIN=/path/to/federated ./tests/e2e/run_all.sh
```

## Test Coverage

### SMTP Service Tests
- Start with default configuration
- Start with custom port
- Environment variable configuration
- Environment file configuration
- Service stop
- Logging
- Multiple restarts

### IMAP Service Tests
- Start with default configuration
- Start with custom port
- Environment variable configuration
- Service stop
- Different log levels
- No-color output

### Combined Service Tests
- SMTP and IMAP running together
- Sequential service management
- Shared mail store
- Different configurations per service
- Service restart while others run
- File logging for multiple services

## Test Framework Features

### Assertions
- `assert_equals` - Compare two values
- `assert_contains` - Check if string contains substring
- `assert_file_exists` - Verify file existence
- `assert_process_running` - Check if process is running

### Service Management
- `start_service` - Start a service in background
- `stop_service` - Stop a running service
- `get_service_log` - Retrieve service logs

### Logging
- `log_info` - Information messages
- `log_success` - Success messages
- `log_error` - Error messages
- `log_warn` - Warning messages

## Requirements

- Bash 4.0+
- Built `federated` binary
- Standard Unix utilities (nc, kill, etc.)

## CI Integration

These tests are automatically run in the CI pipeline after unit tests:

1. Unit tests (federated_tests)
2. CTest
3. E2E tests

## Adding New Tests

1. Create a new test function in the appropriate file
2. Add the test to the main() function
3. Use the test framework assertions and utilities
4. Follow the naming convention: `test_<service>_<scenario>`

Example:

```bash
test_smtp_custom_feature() {
    log_info "Testing custom SMTP feature"
    
    start_service smtp --port 2525 --custom-option
    
    local log=$(get_service_log smtp)
    assert_contains "$log" "Custom feature enabled" "Should enable custom feature"
    
    stop_service smtp
}
```

## Notes

- Tests run in isolated temporary directories
- All background processes are cleaned up on exit
- Logs are preserved in temp directory during test run
- Tests are designed to be idempotent and isolated
