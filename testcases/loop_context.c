/**
 * Validation Test: Loop Context
 *
 * Purpose: Test that ICHA correctly identifies conversions in loop conditions
 * Expected: All conversions should be detected as LOOP_CONTEXT
 */

int main() {
    // Test 1: Float to int in for loop condition
    double size = 100.5;
    for (int i = 0; i < size; i++) {  // Expected: LOOP_CONTEXT
        // Loop body
    }

    // Test 2: Long to int in while loop condition
    long max_iterations = 1000L;
    int counter = 0;
    while (counter < max_iterations) {  // Expected: LOOP_CONTEXT
        counter++;
    }

    // Test 3: Float in for loop initialization
    float start = 0.0f;
    for (int j = start; j < 10; j++) {  // Expected: LOOP_CONTEXT
        // Loop body
    }

    // Test 4: Double in do-while condition
    double threshold = 50.7;
    int value = 0;
    do {
        value++;
    } while (value < threshold);  // Expected: LOOP_CONTEXT

    // Test 5: Unsigned to signed in loop condition
    unsigned int limit = 100U;
    for (int k = 0; k < limit; k++) {  // Expected: LOOP_CONTEXT
        // Loop body
    }

    return 0;
}
