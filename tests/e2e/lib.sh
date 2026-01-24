#!/bin/bash

# E2E Test Framework for Federated
# Provides utilities for testing services end-to-end

set -e

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Test counters
TESTS_RUN=0
TESTS_PASSED=0
TESTS_FAILED=0

# Configuration
FEDERATED_BIN="${FEDERATED_BIN:-./build/federated}"
TEST_TIMEOUT="${TEST_TIMEOUT:-30}"
TEMP_DIR=$(mktemp -d)
LOG_DIR="${TEMP_DIR}/logs"
mkdir -p "$LOG_DIR"

# Cleanup on exit
cleanup() {
    local exit_code=$?
    
    # Kill any remaining background processes
    jobs -p | xargs -r kill 2>/dev/null || true
    
    # Clean up temp directory
    rm -rf "$TEMP_DIR"
    
    exit $exit_code
}
trap cleanup EXIT INT TERM

# Logging functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $*"
}

log_success() {
    echo -e "${GREEN}[PASS]${NC} $*"
}

log_error() {
    echo -e "${RED}[FAIL]${NC} $*"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $*"
}

# Test assertions
assert_equals() {
    local expected="$1"
    local actual="$2"
    local message="${3:-Assertion failed}"
    
    if [ "$expected" = "$actual" ]; then
        return 0
    else
        log_error "$message"
        log_error "  Expected: '$expected'"
        log_error "  Actual:   '$actual'"
        return 1
    fi
}

assert_contains() {
    local haystack="$1"
    local needle="$2"
    local message="${3:-String not found}"
    
    if echo "$haystack" | grep -q "$needle"; then
        return 0
    else
        log_error "$message"
        log_error "  Looking for: '$needle'"
        log_error "  In: '$haystack'"
        return 1
    fi
}

assert_file_exists() {
    local file="$1"
    local message="${2:-File does not exist: $file}"
    
    if [ -f "$file" ]; then
        return 0
    else
        log_error "$message"
        return 1
    fi
}

assert_process_running() {
    local pid="$1"
    local message="${2:-Process not running}"
    
    if kill -0 "$pid" 2>/dev/null; then
        return 0
    else
        log_error "$message (PID: $pid)"
        return 1
    fi
}

# Service management
start_service() {
    local service_name="$1"
    shift
    local extra_args="$@"
    
    local log_file="$LOG_DIR/${service_name}.log"
    local pid_file="$TEMP_DIR/${service_name}.pid"
    
    log_info "Starting $service_name service..."
    
    # Start service in background
    $FEDERATED_BIN service start "$service_name" $extra_args \
        --log-file "$log_file" --no-color > "$log_file" 2>&1 &
    
    local pid=$!
    echo "$pid" > "$pid_file"
    
    # Wait for service to start
    sleep 2
    
    if kill -0 "$pid" 2>/dev/null; then
        log_info "$service_name started (PID: $pid)"
        return 0
    else
        log_error "Failed to start $service_name"
        cat "$log_file"
        return 1
    fi
}

stop_service() {
    local service_name="$1"
    local pid_file="$TEMP_DIR/${service_name}.pid"
    
    if [ -f "$pid_file" ]; then
        local pid=$(cat "$pid_file")
        if kill -0 "$pid" 2>/dev/null; then
            log_info "Stopping $service_name (PID: $pid)..."
            kill "$pid" 2>/dev/null || true
            sleep 1
            kill -9 "$pid" 2>/dev/null || true
        fi
        rm -f "$pid_file"
    fi
}

get_service_log() {
    local service_name="$1"
    local log_file="$LOG_DIR/${service_name}.log"
    
    if [ -f "$log_file" ]; then
        cat "$log_file"
    fi
}

# Test framework
run_test() {
    local test_name="$1"
    local test_func="$2"
    
    TESTS_RUN=$((TESTS_RUN + 1))
    
    echo ""
    echo "========================================="
    echo "Test: $test_name"
    echo "========================================="
    
    # Run test in subshell to isolate environment
    if (
        set -e
        $test_func
    ); then
        TESTS_PASSED=$((TESTS_PASSED + 1))
        log_success "$test_name"
        return 0
    else
        TESTS_FAILED=$((TESTS_FAILED + 1))
        log_error "$test_name"
        return 1
    fi
}

# Test summary
print_summary() {
    echo ""
    echo "========================================="
    echo "Test Summary"
    echo "========================================="
    echo "Total:  $TESTS_RUN"
    echo -e "${GREEN}Passed: $TESTS_PASSED${NC}"
    echo -e "${RED}Failed: $TESTS_FAILED${NC}"
    echo "========================================="
    
    if [ $TESTS_FAILED -eq 0 ]; then
        echo -e "${GREEN}All tests passed!${NC}"
        return 0
    else
        echo -e "${RED}Some tests failed!${NC}"
        return 1
    fi
}

# Utility functions
wait_for_port() {
    local port="$1"
    local timeout="${2:-10}"
    local start_time=$(date +%s)
    
    while ! nc -z localhost "$port" 2>/dev/null; do
        local current_time=$(date +%s)
        local elapsed=$((current_time - start_time))
        
        if [ $elapsed -ge $timeout ]; then
            log_error "Timeout waiting for port $port"
            return 1
        fi
        
        sleep 0.5
    done
    
    return 0
}

# Export functions for use in test scripts
export -f log_info log_success log_error log_warn
export -f assert_equals assert_contains assert_file_exists assert_process_running
export -f start_service stop_service get_service_log
export -f run_test print_summary
export -f wait_for_port
