#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════════
# Implicit Conversion Hazard Analyzer — Context Validation Test Suite
# ═══════════════════════════════════════════════════════════════════════════════

ICHA_BINARY="./build/src/icha"
TEST_DIR="testcases"
PASS_COUNT=0
FAIL_COUNT=0
TOTAL=0

# macOS SDK flags
EXTRA_FLAGS="-- -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"

echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                    IMPLICIT CONVERSION HAZARD ANALYZER                     ║"
echo "║                         VALIDATION TEST SUITE                              ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""
echo "Running validation tests..."
echo ""

run_test() {
    local file="$1"
    local expected_context="$2"
    local test_name=$(basename "$file" .c)
    TOTAL=$((TOTAL + 1))

    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "Test: $test_name"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

    output=$($ICHA_BINARY "$file" $EXTRA_FLAGS 2>&1)
    finding_count=$(echo "$output" | grep -c "Primary Context:")
    echo "Findings: $finding_count"
    echo ""

    echo "Contexts detected:"
    echo "$output" | grep "Primary Context:" | sort | uniq -c
    echo "$output" | grep "Context:" | grep -v "Primary" | sort | uniq -c
    echo ""

    if echo "$output" | grep -q "$expected_context"; then
        echo "✅ PASSED: Expected context '$expected_context' found"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        echo "❌ FAILED: Expected context '$expected_context' NOT found"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
    echo ""
}

# Run all context validation tests
run_test "$TEST_DIR/assignment_context.c" "ASSIGNMENT_CONTEXT"
run_test "$TEST_DIR/loop_context.c"       "LOOP_CONTEXT"
run_test "$TEST_DIR/memory_context.c"     "MEMORY_CONTEXT"
run_test "$TEST_DIR/branch_context.c"     "BRANCH_CONTEXT"
run_test "$TEST_DIR/api_context.c"        "API_CONTEXT"
run_test "$TEST_DIR/return_context.c"     "RETURN_CONTEXT"

echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                           TEST SUMMARY                                     ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""
echo "Total Tests:  $TOTAL"
echo "Passed:       $PASS_COUNT"
echo "Failed:       $FAIL_COUNT"
echo ""

if [ $FAIL_COUNT -eq 0 ]; then
    echo "🎉 All validation tests passed!"
    exit 0
else
    echo "⚠️  Some tests failed. Please review."
    exit 1
fi
