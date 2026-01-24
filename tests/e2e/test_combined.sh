#!/bin/bash

# E2E Tests for Combined Services (SMTP + IMAP)

set -e

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib.sh"

# Test: Start both SMTP and IMAP services
test_smtp_imap_together() {
    log_info "Testing SMTP and IMAP services running together"
    
    # Start SMTP
    start_service smtp --port 2525
    local smtp_log=$(get_service_log smtp)
    assert_contains "$smtp_log" "SMTP service started successfully" "SMTP should start"
    
    # Start IMAP
    start_service imap --port 1143
    local imap_log=$(get_service_log imap)
    assert_contains "$imap_log" "IMAP service started successfully" "IMAP should start"
    
    # Verify both are running
    local smtp_pid=$(cat "$TEMP_DIR/smtp.pid")
    local imap_pid=$(cat "$TEMP_DIR/imap.pid")
    
    assert_process_running "$smtp_pid" "SMTP should be running"
    assert_process_running "$imap_pid" "IMAP should be running"
    
    # Stop both
    stop_service smtp
    stop_service imap
    
    log_success "Both services ran together successfully"
}

# Test: Sequential start/stop of services
test_sequential_services() {
    log_info "Testing sequential service management"
    
    # Start SMTP
    start_service smtp --port 3025
    sleep 1
    
    # Start IMAP while SMTP is running
    start_service imap --port 2143
    sleep 1
    
    # Stop SMTP first
    stop_service smtp
    sleep 1
    
    # Verify IMAP still running
    local imap_pid=$(cat "$TEMP_DIR/imap.pid")
    assert_process_running "$imap_pid" "IMAP should still be running"
    
    # Stop IMAP
    stop_service imap
    
    log_success "Sequential service management successful"
}

# Test: Services with shared mail store
test_shared_mail_store() {
    log_info "Testing services with shared mail store"
    
    # Both services should be able to access the same mail store
    start_service smtp --port 4025
    start_service imap --port 3143
    
    # Both services initialize the mail store
    local smtp_log=$(get_service_log smtp)
    local imap_log=$(get_service_log imap)
    
    assert_contains "$smtp_log" "started successfully" "SMTP should initialize store"
    assert_contains "$imap_log" "started successfully" "IMAP should initialize store"
    
    stop_service smtp
    stop_service imap
    
    log_success "Shared mail store test passed"
}

# Test: Different configurations for each service
test_different_configs() {
    log_info "Testing services with different configurations"
    
    # Create separate env files
    local smtp_env="$TEMP_DIR/smtp.env"
    local imap_env="$TEMP_DIR/imap.env"
    
    cat > "$smtp_env" << EOF
FEDERATED_SMTP_PORT=5025
FEDERATED_SMTP_HOST=127.0.0.1
EOF
    
    cat > "$imap_env" << EOF
FEDERATED_IMAP_PORT=4143
FEDERATED_IMAP_HOST=127.0.0.1
EOF
    
    start_service smtp --env-file "$smtp_env"
    start_service imap --env-file "$imap_env"
    
    local smtp_pid=$(cat "$TEMP_DIR/smtp.pid")
    local imap_pid=$(cat "$TEMP_DIR/imap.pid")
    
    assert_process_running "$smtp_pid" "SMTP should be running"
    assert_process_running "$imap_pid" "IMAP should be running"
    
    stop_service smtp
    stop_service imap
    
    log_success "Different configurations test passed"
}

# Test: Service restart while other is running
test_service_restart() {
    log_info "Testing service restart while other remains running"
    
    start_service smtp --port 6025
    start_service imap --port 5143
    
    # Get IMAP PID
    local imap_pid_before=$(cat "$TEMP_DIR/imap.pid")
    
    # Restart SMTP
    stop_service smtp
    sleep 1
    start_service smtp --port 6025
    
    # Verify IMAP is still running with same PID
    local imap_pid_after=$(cat "$TEMP_DIR/imap.pid")
    assert_equals "$imap_pid_before" "$imap_pid_after" "IMAP should not be affected by SMTP restart"
    
    stop_service smtp
    stop_service imap
    
    log_success "Service restart test passed"
}

# Test: All services with logging to file
test_services_file_logging() {
    log_info "Testing services with file logging"
    
    local smtp_log_file="$TEMP_DIR/smtp_service.log"
    local imap_log_file="$TEMP_DIR/imap_service.log"
    
    start_service smtp --port 7025 --log-file "$smtp_log_file"
    start_service imap --port 6143 --log-file "$imap_log_file"
    
    sleep 2
    
    assert_file_exists "$smtp_log_file" "SMTP log file should exist"
    assert_file_exists "$imap_log_file" "IMAP log file should exist"
    
    local smtp_content=$(cat "$smtp_log_file")
    local imap_content=$(cat "$imap_log_file")
    
    assert_contains "$smtp_content" "SMTP" "SMTP log should contain service name"
    assert_contains "$imap_content" "IMAP" "IMAP log should contain service name"
    
    stop_service smtp
    stop_service imap
    
    log_success "File logging test passed"
}

# Run all tests
main() {
    log_info "Starting Combined Services E2E Tests"
    log_info "Using binary: $FEDERATED_BIN"
    
    if [ ! -x "$FEDERATED_BIN" ]; then
        log_error "Federated binary not found or not executable: $FEDERATED_BIN"
        exit 1
    fi
    
    run_test "SMTP + IMAP Together" test_smtp_imap_together
    run_test "Sequential Services" test_sequential_services
    run_test "Shared Mail Store" test_shared_mail_store
    run_test "Different Configs" test_different_configs
    run_test "Service Restart" test_service_restart
    run_test "Services File Logging" test_services_file_logging
    
    print_summary
}

main
