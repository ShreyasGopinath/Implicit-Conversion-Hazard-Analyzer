/**
 * Validation Test: API Context
 *
 * Purpose: Test that ICHA correctly identifies conversions at API boundaries
 * Expected: All conversions should be detected as API_CONTEXT or function call context
 */

// External API functions (declarations)
void process_data(unsigned int size);
int calculate_result(int value);
void set_temperature(int temp);
void handle_index(unsigned int idx);

int main() {
    // Test 1: Signed to unsigned in function call
    int signed_size = -10;
    process_data(signed_size);  // Expected: API_CONTEXT (DANGEROUS!)

    // Test 2: Float to int in function call
    float temperature = 98.6f;
    set_temperature(temperature);  // Expected: API_CONTEXT

    // Test 3: Long to int in function call
    long large_value = 1000000L;
    calculate_result(large_value);  // Expected: API_CONTEXT

    // Test 4: Double to unsigned in function call
    double index = 42.7;
    handle_index(index);  // Expected: API_CONTEXT

    // Test 5: Unsigned to signed in function call
    unsigned int unsigned_val = 4000000000U;
    calculate_result(unsigned_val);  // Expected: API_CONTEXT

    return 0;
}

// Function definitions (to make it compile)
void process_data(unsigned int size) {
    // Function body
}

int calculate_result(int value) {
    return value * 2;
}

void set_temperature(int temp) {
    // Function body
}

void handle_index(unsigned int idx) {
    // Function body
}
