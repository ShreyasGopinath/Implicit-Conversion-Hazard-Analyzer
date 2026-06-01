/**
 * Validation Test: Assignment Context
 *
 * Purpose: Test that ICHA correctly identifies conversions in assignment context
 * Expected: All conversions should be detected as ASSIGNMENT_CONTEXT
 */

int main() {
    // Test 1: Simple assignment - signed to unsigned
    int signed_val = -10;
    unsigned int unsigned_val = signed_val;  // Expected: ASSIGNMENT_CONTEXT

    // Test 2: Float to int assignment
    float temperature = 98.6f;
    int temp_int = temperature;  // Expected: ASSIGNMENT_CONTEXT

    // Test 3: Long to int assignment
    long large_number = 1000000L;
    int small_number = large_number;  // Expected: ASSIGNMENT_CONTEXT

    // Test 4: Double to float assignment
    double precise = 3.141592653589793;
    float less_precise = precise;  // Expected: ASSIGNMENT_CONTEXT

    // Test 5: Unsigned to signed assignment
    unsigned int big_unsigned = 4000000000U;
    int signed_result = big_unsigned;  // Expected: ASSIGNMENT_CONTEXT

    return 0;
}
