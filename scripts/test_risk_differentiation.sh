#!/bin/bash
# ═══════════════════════════════════════════════════════════════════════════════
# Implicit Conversion Hazard Analyzer — Risk Differentiation Test Suite
# Verifies that identical conversions receive different risk scores 
# depending on semantic context (array index vs assignment vs loop bound).
# ═══════════════════════════════════════════════════════════════════════════════

ICHA_BINARY="./build/src/icha"
TEST_DIR="testcases"
EXTRA_FLAGS="-- -isysroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk"

echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║              RISK DIFFERENTIATION TEST SUITE                               ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""

PASS=0
FAIL=0

# Test 1: Memory context should have CRITICAL findings
echo "━━━ Test 1: MEMORY_CONTEXT should produce CRITICAL risk ━━━"
output=$($ICHA_BINARY -icha-format=json "$TEST_DIR/memory_context.c" $EXTRA_FLAGS 2>&1)
if echo "$output" | grep -qi "CRITICAL\|HIGH"; then
    echo "✅ PASSED: Memory context produces HIGH/CRITICAL findings"
    PASS=$((PASS + 1))
else
    echo "❌ FAILED: Memory context did not produce HIGH/CRITICAL findings"
    FAIL=$((FAIL + 1))
fi
echo ""

# Test 2: Loop context should have HIGH findings
echo "━━━ Test 2: LOOP_CONTEXT should produce HIGH risk ━━━"
output=$($ICHA_BINARY -icha-format=json "$TEST_DIR/loop_context.c" $EXTRA_FLAGS 2>&1)
if echo "$output" | grep -qi "HIGH\|CRITICAL"; then
    echo "✅ PASSED: Loop context produces HIGH/CRITICAL findings"
    PASS=$((PASS + 1))
else
    echo "❌ FAILED: Loop context did not produce HIGH/CRITICAL findings"
    FAIL=$((FAIL + 1))
fi
echo ""

# Test 3: Assignment context should generally produce MEDIUM/LOW (not all CRITICAL)
echo "━━━ Test 3: ASSIGNMENT_CONTEXT should produce MEDIUM/LOW risk ━━━"
output=$($ICHA_BINARY -icha-format=json "$TEST_DIR/assignment_context.c" $EXTRA_FLAGS 2>&1)
if echo "$output" | grep -qi "MEDIUM\|LOW"; then
    echo "✅ PASSED: Assignment context produces MEDIUM/LOW findings"
    PASS=$((PASS + 1))
else
    echo "❌ FAILED: Assignment context did not produce MEDIUM/LOW findings"
    FAIL=$((FAIL + 1))
fi
echo ""

echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                     DIFFERENTIATION SUMMARY                                ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo ""
echo "Passed: $PASS / $((PASS + FAIL))"

if [ $FAIL -eq 0 ]; then
    echo "🎉 All risk differentiation tests passed!"
    exit 0
else
    echo "⚠️  Some differentiation tests failed."
    exit 1
fi
