/**
 * Test Case: API Boundary Conversions
 *
 * Tests implicit conversions at function/API boundaries
 * Expected: HIGH/CRITICAL findings for external interfaces
 */

#include <cstddef>

// External API declarations
extern void set_buffer_size(unsigned int size);
extern int process_data(int count);
extern void configure_timeout(int milliseconds);
extern size_t get_file_size();

// HIGH: Sign change at API boundary
void test_api_sign_change() {
    int size = -100;

    // HIGH: Negative value passed to unsigned parameter
    // Risk: API receives unexpected large value
    set_buffer_size(size);
}

// HIGH: Narrowing at API boundary
void test_api_narrowing() {
    long long large_count = 0x100000000LL;

    // HIGH: Value truncated when passed to API
    // Risk: API receives incorrect value
    int result = process_data(large_count);
}

// MEDIUM: Widening at API boundary (safer)
void test_api_widening() {
    int timeout = 5000;

    // MEDIUM: Widening conversion (generally safe)
    configure_timeout(timeout);
}

// HIGH: Return value conversion
void test_api_return_conversion() {
    // HIGH: Return value may be truncated
    // Risk: Incorrect file size
    int file_size = get_file_size();
}

// HIGH: Multiple conversions in API call chain
void test_api_chain_conversions() {
    unsigned int count = 0xFFFFFFFFU;

    // HIGH: Conversion in parameter passing
    int signed_count = count;
    int result = process_data(signed_count);

    // HIGH: Conversion in return value
    unsigned int final_result = result;
}

// CRITICAL: Conversion in callback parameter
typedef void (*callback_t)(unsigned int value);

void register_callback(callback_t cb);

void my_callback(unsigned int value) {
    // Process value
}

void test_callback_conversion() {
    int negative_value = -50;

    // CRITICAL: Negative value passed through callback
    // Risk: Callback receives unexpected value
    register_callback([](unsigned int val) {
        // val is now a large positive number
    });
}

// HIGH: Conversion in struct member at API boundary
struct Config {
    unsigned int buffer_size;
    int timeout;
};

extern void apply_config(const Config& cfg);

void test_struct_api_conversion() {
    int size = -1000;
    long long timeout = 0x100000000LL;

    Config cfg;
    // HIGH: Conversions when initializing struct
    cfg.buffer_size = size;      // Sign change
    cfg.timeout = timeout;       // Narrowing

    apply_config(cfg);
}

int main() {
    test_api_sign_change();
    test_api_narrowing();
    test_api_widening();
    test_api_return_conversion();
    test_api_chain_conversions();
    test_callback_conversion();
    test_struct_api_conversion();
    return 0;
}
