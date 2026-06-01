/**
 * Test Case: Narrowing Conversions
 *
 * Tests implicit narrowing conversions (larger type -> smaller type)
 * Expected: HIGH/CRITICAL findings for data loss scenarios
 */

#include <stdint.h>

// CRITICAL: Long to int in array indexing
void test_narrowing_array_index() {
    int arr[100];
    int64_t large_index = 0x100000000LL;  // Value > INT_MAX

    // CRITICAL: Narrowing conversion loses high bits
    // Risk: Array access with truncated index
    int idx = large_index;
    int value = arr[idx];
}

// HIGH: Double to int in loop condition
void test_narrowing_loop_bound() {
    double limit = 10.9;

    // HIGH: Narrowing conversion loses fractional part
    // Risk: Loop executes 10 times instead of expected 11
    for (int i = 0; i < limit; i++) {
        // Loop body
    }
}

// HIGH: Float to int in calculation
void test_narrowing_calculation() {
    float temperature = 98.6f;

    // HIGH: Narrowing conversion loses precision
    // Risk: Incorrect calculation results
    int temp_int = temperature;
    int adjusted = temp_int * 2;
}

// MEDIUM: Double to float
void test_narrowing_precision_loss() {
    double precise_value = 3.141592653589793;

    // MEDIUM: Narrowing conversion loses precision
    float less_precise = precise_value;
}

// CRITICAL: Long to int at API boundary
extern void set_file_size(int size);

void test_narrowing_api_boundary() {
    int64_t file_size = 0x200000000LL;  // 8GB

    // CRITICAL: Narrowing conversion at API boundary
    // Risk: API receives truncated value
    set_file_size(file_size);
}

// HIGH: Size_t to int conversion
void test_narrowing_size_type() {
    size_t buffer_size = 0xFFFFFFFFULL;

    // HIGH: Narrowing conversion from size_t
    // Risk: Size value truncated
    int size = buffer_size;
}

// CRITICAL: Narrowing in memory operation
void test_narrowing_memory_copy() {
    int64_t count = 0x100000000LL;
    char src[100], dst[100];

    // CRITICAL: Narrowing conversion in memory operation
    // Risk: Incorrect number of bytes copied
    int bytes = count;
    for (int i = 0; i < bytes && i < 100; i++) {
        dst[i] = src[i];
    }
}

int main() {
    test_narrowing_array_index();
    test_narrowing_loop_bound();
    test_narrowing_calculation();
    test_narrowing_precision_loss();
    test_narrowing_api_boundary();
    test_narrowing_size_type();
    test_narrowing_memory_copy();
    return 0;
}
