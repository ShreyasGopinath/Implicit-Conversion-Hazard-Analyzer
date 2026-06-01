# Quick Guide: Using the Validation Test Suite

## What It Is

A set of 6 test files that validate ICHA's ability to detect implicit conversions in different contexts.

## Quick Start

```bash
cd "/Users/shreyasgopinath/Desktop/CD Lab EL"

# Run all tests
./run_validation_tests.sh

# Or run individual tests
./build/src/icha tests/validation/assignment_context.c --
./build/src/icha tests/validation/loop_context.c --
./build/src/icha tests/validation/memory_context.c --
```

## Test Files

1. **assignment_context.c** - Simple assignments (5 conversions)
2. **loop_context.c** - Loop conditions (5 conversions)
3. **memory_context.c** - Array indexing (7 conversions)
4. **branch_context.c** - If/else conditions (4 conversions)
5. **api_context.c** - Function arguments (5 conversions)
6. **return_context.c** - Return statements (6 conversions)

**Total**: 32 conversions across 6 files

## Expected Results

All tests should detect conversions and show:
- ✅ Total findings count
- ✅ Risk scores (0-100)
- ✅ Severity levels
- ✅ Fix suggestions

## What This Validates

✅ ICHA detects implicit conversions  
✅ Risk scoring works  
✅ Fix suggestions are generated  
✅ Multiple output formats work  

## Documentation

- **README.md** - Detailed test suite documentation
- **VALIDATION_RESULTS.md** - Complete test results and analysis

## Quick Test

```bash
# Test one file to verify it works
./build/src/icha tests/validation/simple_test.c --
```

Expected: Should show 5 findings

---

**Created**: June 1, 2026  
**Status**: ✅ Working
