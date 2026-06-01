/**
 * Test Case: Loop Context Conversions
 *
 * Tests implicit conversions in loop-related contexts
 * Expected: HIGH findings for loop bounds and conditions
 */

// HIGH: Float to int in loop bound
void test_float_loop_bound() {
    float max_iterations = 100.5f;

    // HIGH: Implicit conversion in loop condition
    // Risk: Loop executes 100 times, not 101
    for (int i = 0; i < max_iterations; i++) {
        // Process
    }
}

// HIGH: Unsigned to signed in loop
void test_unsigned_loop_bound() {
    unsigned int count = 4000000000U;  // > INT_MAX

    // HIGH: Implicit conversion in loop condition
    // Risk: Comparison may not work as expected
    for (int i = 0; i < count; i++) {
        // Process
    }
}

// CRITICAL: Negative value in loop bound
void test_negative_loop_bound() {
    int limit = -10;

    // CRITICAL: Implicit conversion to unsigned
    // Risk: Loop executes huge number of times
    for (unsigned int i = 0; i < limit; i++) {
        // Infinite-like loop!
    }
}

// HIGH: Double to int in loop increment
void test_double_loop_increment() {
    double step = 0.5;

    // HIGH: Implicit conversion in loop increment
    // Risk: Step becomes 0, infinite loop
    for (int i = 0; i < 100; i += step) {
        // May not terminate
    }
}

// MEDIUM: Widening in loop (less dangerous)
void test_widening_loop() {
    int limit = 100;

    // MEDIUM: Widening conversion (safer)
    for (long i = 0; i < limit; i++) {
        // Generally safe
    }
}

// HIGH: Conversion in nested loop
void test_nested_loop_conversion() {
    float outer_limit = 10.9f;
    double inner_limit = 20.5;

    // HIGH: Multiple conversions in nested loops
    for (int i = 0; i < outer_limit; i++) {
        for (int j = 0; j < inner_limit; j++) {
            // Nested loop with conversions
        }
    }
}

int main() {
    test_float_loop_bound();
    test_unsigned_loop_bound();
    test_negative_loop_bound();
    test_double_loop_increment();
    test_widening_loop();
    test_nested_loop_conversion();
    return 0;
}
