#!/bin/bash

# Master E2E Test Runner
# Runs all end-to-end tests for Federated services

set -e

# Get the directory of this script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Color codes
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
export FEDERATED_BIN="${FEDERATED_BIN:-$SCRIPT_DIR/../../build/federated}"
export TEST_TIMEOUT="${TEST_TIMEOUT:-30}"

# Test suite counters
SUITES_RUN=0
SUITES_PASSED=0
SUITES_FAILED=0

log_header() {
    echo ""
    echo "========================================="
    echo "$1"
    echo "========================================="
}

run_test_suite() {
    local suite_name="$1"
    local suite_script="$2"
    
    SUITES_RUN=$((SUITES_RUN + 1))
    
    log_header "Test Suite: $suite_name"
    
    if bash "$suite_script"; then
        SUITES_PASSED=$((SUITES_PASSED + 1))
        echo -e "${GREEN}✓ $suite_name PASSED${NC}"
        return 0
    else
        SUITES_FAILED=$((SUITES_FAILED + 1))
        echo -e "${RED}✗ $suite_name FAILED${NC}"
        return 1
    fi
}

main() {
    log_header "Federated E2E Test Suite"
    
    echo "Configuration:"
    echo "  Binary: $FEDERATED_BIN"
    echo "  Timeout: ${TEST_TIMEOUT}s"
    echo ""
    
    # Check if binary exists
    if [ ! -x "$FEDERATED_BIN" ]; then
        echo -e "${RED}Error: Federated binary not found or not executable${NC}"
        echo "  Path: $FEDERATED_BIN"
        echo ""
        echo "Please build the project first:"
        echo "  mkdir build && cd build"
        echo "  cmake .. && make"
        exit 1
    fi
    
    # Run test suites
    run_test_suite "SMTP Service" "$SCRIPT_DIR/test_smtp.sh" || true
    run_test_suite "IMAP Service" "$SCRIPT_DIR/test_imap.sh" || true
    run_test_suite "Combined Services" "$SCRIPT_DIR/test_combined.sh" || true
    
    # Print summary
    echo ""
    log_header "E2E Test Summary"
    echo "Total Suites: $SUITES_RUN"
    echo -e "${GREEN}Passed:       $SUITES_PASSED${NC}"
    echo -e "${RED}Failed:       $SUITES_FAILED${NC}"
    echo "========================================="
    
    if [ $SUITES_FAILED -eq 0 ]; then
        echo -e "${GREEN}All test suites passed!${NC}"
        exit 0
    else
        echo -e "${RED}Some test suites failed!${NC}"
        exit 1
    fi
}

main "$@"
