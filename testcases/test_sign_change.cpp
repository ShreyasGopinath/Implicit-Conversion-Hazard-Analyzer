/**
 * Test Case: Sign Change Conversions
 *
 * Tests implicit conversions between signed and unsigned types
 * Expected: CRITICAL findings for memory/array contexts
 */

#include <cstddef>

// CRITICAL: Signed to unsigned in array indexing
void test_array_index_sign_change() {
    int arr[10];
    int index = -1;  // Negative value

    // CRITICAL: Implicit conversion int -> unsigned in array access
    // Risk: Negative value becomes large positive, buffer overflow
    unsigned int idx = index;
    int value = arr[idx];  // Dangerous!
}

// HIGH: Signed to unsigned in loop condition
void test_loop_sign_change() {
    int limit = -5;

    // HIGH: Implicit conversion in loop bound
    // Risk: Loop may not execute as expected
    for (unsigned int i = 0; i < limit; i++) {
        // Loop body
    }
}

// HIGH: Unsigned to signed comparison
void test_comparison_sign_change() {
    unsigned int size = 100;
    int threshold = -10;

    // HIGH: Implicit conversion in comparison
    // Risk: Unexpected comparison results
    if (size > threshold) {
        // May not behave as expected
    }
}

// MEDIUM: Sign change in assignment
void test_assignment_sign_change() {
    int signed_value = -42;

    // MEDIUM: Implicit conversion in simple assignment
    unsigned int unsigned_value = signed_value;
}

// CRITICAL: Sign change in memory allocation
void test_memory_allocation_sign_change() {
    int size = -10;

    // CRITICAL: Implicit conversion in size calculation
    // Risk: Negative size becomes huge positive value
    char* buffer = new char[size];
    delete[] buffer;
}

// HIGH: Sign change at API boundary
extern void process_data(unsigned int count);

void test_api_sign_change() {
    int count = -5;

    // HIGH: Implicit conversion at API boundary
    // Risk: API receives unexpected value
    process_data(count);
}

int main() {
    test_array_index_sign_change();
    test_loop_sign_change();
    test_comparison_sign_change();
    test_assignment_sign_change();
    test_memory_allocation_sign_change();
    test_api_sign_change();
    return 0;
}
