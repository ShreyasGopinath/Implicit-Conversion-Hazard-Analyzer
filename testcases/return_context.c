/**
 * Validation Test: Return Context
 *
 * Purpose: Test that ICHA correctly identifies conversions in return statements
 * Expected: All conversions should be detected as RETURN_CONTEXT
 */

// Test 1: Float to int return
int get_temperature() {
    float temp = 98.6f;
    return temp;  // Expected: RETURN_CONTEXT
}

// Test 2: Long to int return
int get_count() {
    long large_count = 1000000L;
    return large_count;  // Expected: RETURN_CONTEXT
}

// Test 3: Double to int return
int get_rounded_value() {
    double precise_value = 42.789;
    return precise_value;  // Expected: RETURN_CONTEXT
}

// Test 4: Unsigned to signed return
int get_size() {
    unsigned int unsigned_size = 4000000000U;
    return unsigned_size;  // Expected: RETURN_CONTEXT (DANGEROUS!)
}

// Test 5: Signed to unsigned return
unsigned int get_index() {
    int signed_index = -5;
    return signed_index;  // Expected: RETURN_CONTEXT (VERY DANGEROUS!)
}

// Test 6: Double to float return
float get_pi() {
    double precise_pi = 3.141592653589793;
    return precise_pi;  // Expected: RETURN_CONTEXT
}

int main() {
    // Call all functions to make them used
    int t = get_temperature();
    int c = get_count();
    int r = get_rounded_value();
    int s = get_size();
    unsigned int i = get_index();
    float p = get_pi();

    return 0;
}
