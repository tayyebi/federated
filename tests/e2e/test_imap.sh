#!/bin/bash

# E2E Tests for IMAP Service

set -e

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib.sh"

# Test: IMAP service can start with default configuration
test_imap_start_default() {
    log_info "Testing IMAP service start with defaults"
    
    start_service imap
    
    local log=$(get_service_log imap)
    assert_contains "$log" "Starting IMAP Service" "Should log startup message"
    assert_contains "$log" "IMAP service started successfully" "Should start successfully"
    
    stop_service imap
}

# Test: IMAP service can start with custom port
test_imap_start_custom_port() {
    log_info "Testing IMAP service with custom port"
    
    start_service imap --port 1143
    
    local log=$(get_service_log imap)
    assert_contains "$log" "Port: 1143" "Should use custom port"
    
    stop_service imap
}

# Test: IMAP service with environment variables
test_imap_env_vars() {
    log_info "Testing IMAP service with environment variables"
    
    export FEDERATED_IMAP_PORT=2143
    export FEDERATED_IMAP_HOST="127.0.0.1"
    
    start_service imap
    
    local log=$(get_service_log imap)
    assert_contains "$log" "Port: 2143" "Should use env var port"
    
    stop_service imap
    
    unset FEDERATED_IMAP_PORT
    unset FEDERATED_IMAP_HOST
}

# Test: IMAP service stop
test_imap_stop() {
    log_info "Testing IMAP service stop"
    
    start_service imap --port 3143
    
    local pid_file="$TEMP_DIR/imap.pid"
    assert_file_exists "$pid_file" "PID file should exist"
    
    local pid=$(cat "$pid_file")
    assert_process_running "$pid" "Service should be running"
    
    stop_service imap
    
    sleep 1
    
    if kill -0 "$pid" 2>/dev/null; then
        log_error "Process still running after stop"
        return 1
    fi
    
    log_success "Service stopped successfully"
}

# Test: IMAP service with different log levels
test_imap_log_levels() {
    log_info "Testing IMAP service with different log levels"
    
    for level in error warn info debug; do
        log_info "Testing log level: $level"
        start_service imap --port 4143 --log-level "$level"
        
        local log=$(get_service_log imap)
        assert_contains "$log" "Starting IMAP Service" "Should have log entries"
        
        stop_service imap
        sleep 1
    done
    
    log_success "All log levels tested"
}

# Test: IMAP service with no color output
test_imap_no_color() {
    log_info "Testing IMAP service with --no-color"
    
    start_service imap --port 5143 --no-color
    
    local log=$(get_service_log imap)
    assert_contains "$log" "Starting IMAP Service" "Should have log entries"
    # Check that ANSI codes are not present
    if echo "$log" | grep -q $'\033'; then
        log_error "Log contains ANSI color codes when --no-color was specified"
        return 1
    fi
    
    stop_service imap
    
    log_success "No color output verified"
}

# Run all tests
main() {
    log_info "Starting IMAP E2E Tests"
    log_info "Using binary: $FEDERATED_BIN"
    
    if [ ! -x "$FEDERATED_BIN" ]; then
        log_error "Federated binary not found or not executable: $FEDERATED_BIN"
        exit 1
    fi
    
    run_test "IMAP Start Default" test_imap_start_default
    run_test "IMAP Custom Port" test_imap_start_custom_port
    run_test "IMAP Environment Variables" test_imap_env_vars
    run_test "IMAP Stop" test_imap_stop
    run_test "IMAP Log Levels" test_imap_log_levels
    run_test "IMAP No Color" test_imap_no_color
    
    print_summary
}

main
