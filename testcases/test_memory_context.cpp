/**
 * Test Case: Memory Context Conversions
 *
 * Tests implicit conversions in memory-related operations
 * Expected: CRITICAL findings for array indexing and pointer arithmetic
 */

#include <cstring>

// CRITICAL: Sign change in array indexing
void test_array_index_conversion() {
    int data[100];
    int index = -1;

    // CRITICAL: Negative index becomes large positive
    // Risk: Buffer overflow, memory corruption
    unsigned int idx = index;
    data[idx] = 42;  // Dangerous!
}

// CRITICAL: Narrowing in buffer size
void test_buffer_size_narrowing() {
    long long buffer_size = 0x100000000LL;  // 4GB
    char* buffer = new char[1000];

    // CRITICAL: Size truncated in memory operation
    // Risk: Buffer overflow
    int size = buffer_size;
    memset(buffer, 0, size);

    delete[] buffer;
}

// CRITICAL: Conversion in pointer arithmetic
void test_pointer_arithmetic_conversion() {
    int* ptr = new int[100];
    long long offset = 0x80000000LL;

    // CRITICAL: Offset truncated
    // Risk: Incorrect pointer calculation
    int offset_int = offset;
    int* new_ptr = ptr + offset_int;

    delete[] ptr;
}

// CRITICAL: Sign change in memory copy
void test_memcpy_sign_change() {
    char src[100], dst[100];
    int count = -10;

    // CRITICAL: Negative count becomes huge positive
    // Risk: Buffer overflow in memcpy
    size_t bytes = count;
    memcpy(dst, src, bytes);
}

// HIGH: Conversion in array bounds check
void test_bounds_check_conversion() {
    int arr[10];
    long long index = 5;

    // HIGH: Conversion in bounds checking
    int idx = index;
    if (idx >= 0 && idx < 10) {
        arr[idx] = 0;
    }
}

// CRITICAL: Conversion in dynamic allocation
void test_dynamic_allocation_conversion() {
    long long num_elements = 0x100000000LL;

    // CRITICAL: Element count truncated
    // Risk: Insufficient memory allocated
    int count = num_elements;
    int* array = new int[count];

    delete[] array;
}

// CRITICAL: Unsigned to signed in indexing
void test_unsigned_index_conversion() {
    int data[10];
    unsigned int large_index = 0xFFFFFFFFU;

    // CRITICAL: Large unsigned becomes negative signed
    // Risk: Undefined behavior
    int idx = large_index;
    if (idx >= 0 && idx < 10) {
        data[idx] = 0;
    }
}

int main() {
    test_array_index_conversion();
    test_buffer_size_narrowing();
    test_pointer_arithmetic_conversion();
    test_memcpy_sign_change();
    test_bounds_check_conversion();
    test_dynamic_allocation_conversion();
    test_unsigned_index_conversion();
    return 0;
}
