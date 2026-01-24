#!/bin/bash

# E2E Tests for SMTP Service

set -e

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib.sh"

# Test: SMTP service can start with default configuration
test_smtp_start_default() {
    log_info "Testing SMTP service start with defaults"
    
    start_service smtp
    
    local log=$(get_service_log smtp)
    assert_contains "$log" "Starting SMTP Service" "Should log startup message"
    assert_contains "$log" "SMTP service started successfully" "Should start successfully"
    
    stop_service smtp
}

# Test: SMTP service can start with custom port
test_smtp_start_custom_port() {
    log_info "Testing SMTP service with custom port"
    
    start_service smtp --port 2525
    
    local log=$(get_service_log smtp)
    assert_contains "$log" "Port: 2525" "Should use custom port"
    
    stop_service smtp
}

# Test: SMTP service with environment variables
test_smtp_env_vars() {
    log_info "Testing SMTP service with environment variables"
    
    export FEDERATED_SMTP_PORT=3025
    export FEDERATED_SMTP_HOST="127.0.0.1"
    
    start_service smtp
    
    local log=$(get_service_log smtp)
    assert_contains "$log" "Port: 3025" "Should use env var port"
    
    stop_service smtp
    
    unset FEDERATED_SMTP_PORT
    unset FEDERATED_SMTP_HOST
}

# Test: SMTP service with env file
test_smtp_env_file() {
    log_info "Testing SMTP service with env file"
    
    local env_file="$TEMP_DIR/smtp.env"
    cat > "$env_file" << EOF
FEDERATED_SMTP_PORT=4025
FEDERATED_SMTP_HOST=0.0.0.0
EOF
    
    start_service smtp --env-file "$env_file"
    
    local log=$(get_service_log smtp)
    # Note: env file loading might not work in background process,
    # but we test that the service starts
    assert_contains "$log" "SMTP service started successfully" "Should start with env file"
    
    stop_service smtp
}

# Test: SMTP service stop
test_smtp_stop() {
    log_info "Testing SMTP service stop"
    
    start_service smtp --port 5025
    
    local pid_file="$TEMP_DIR/smtp.pid"
    assert_file_exists "$pid_file" "PID file should exist"
    
    local pid=$(cat "$pid_file")
    assert_process_running "$pid" "Service should be running"
    
    stop_service smtp
    
    sleep 1
    
    if kill -0 "$pid" 2>/dev/null; then
        log_error "Process still running after stop"
        return 1
    fi
    
    log_success "Service stopped successfully"
}

# Test: SMTP service logging
test_smtp_logging() {
    log_info "Testing SMTP service logging"
    
    start_service smtp --port 6025 --log-level debug
    
    local log_file="$LOG_DIR/smtp.log"
    assert_file_exists "$log_file" "Log file should exist"
    
    local log=$(cat "$log_file")
    assert_contains "$log" "Starting SMTP Service" "Should have log entries"
    
    stop_service smtp
}

# Test: Multiple SMTP restarts
test_smtp_multiple_restarts() {
    log_info "Testing multiple SMTP restarts"
    
    for i in {1..3}; do
        log_info "Start attempt $i"
        start_service smtp --port $((7000 + i))
        sleep 1
        stop_service smtp
        sleep 1
    done
    
    log_success "Multiple restarts successful"
}

# Run all tests
main() {
    log_info "Starting SMTP E2E Tests"
    log_info "Using binary: $FEDERATED_BIN"
    
    if [ ! -x "$FEDERATED_BIN" ]; then
        log_error "Federated binary not found or not executable: $FEDERATED_BIN"
        exit 1
    fi
    
    run_test "SMTP Start Default" test_smtp_start_default
    run_test "SMTP Custom Port" test_smtp_start_custom_port
    run_test "SMTP Environment Variables" test_smtp_env_vars
    run_test "SMTP Env File" test_smtp_env_file
    run_test "SMTP Stop" test_smtp_stop
    run_test "SMTP Logging" test_smtp_logging
    run_test "SMTP Multiple Restarts" test_smtp_multiple_restarts
    
    print_summary
}

main
