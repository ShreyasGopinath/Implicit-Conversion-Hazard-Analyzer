/**
 * Validation Test: Memory Context
 *
 * Purpose: Test that ICHA correctly identifies conversions used in array indexing
 * Expected: All conversions should be detected, with high risk for array access
 */

int main() {
    int array[100];

    // Test 1: Float to int conversion, then used as array index (DANGEROUS!)
    double index_double = 5.7;
    int index1 = index_double;  // Implicit conversion here
    array[index1] = 10;

    // Test 2: Long to int conversion, then used as array index
    long index_long = 42L;
    int index2 = index_long;  // Implicit conversion here
    array[index2] = 20;

    // Test 3: Signed to unsigned conversion for array indexing (VERY DANGEROUS!)
    int signed_index = -5;
    unsigned int unsigned_index = signed_index;  // Implicit conversion here
    // array[unsigned_index] = 30;  // Would crash - negative wraps to huge value

    // Test 4: Float conversion for array access
    float position_float = 10.5f;
    int position = position_float;  // Implicit conversion here
    int value = array[position];

    // Test 5: Double conversion for multi-dimensional array
    int matrix[10][10];
    double row_double = 3.8;
    double col_double = 7.2;
    int row = row_double;  // Implicit conversion here
    int col = col_double;  // Implicit conversion here
    matrix[row][col] = 100;

    // Test 6: Unsigned to signed for indexing
    unsigned int big_index = 4000000000U;
    int small_index = big_index;  // Implicit conversion here (DANGEROUS!)
    // array[small_index] = 50;  // Would likely crash

    return 0;
}
