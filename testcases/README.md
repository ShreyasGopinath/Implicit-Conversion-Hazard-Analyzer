# ICHA Validation Test Suite

## Purpose

This test suite validates that ICHA correctly identifies different **context types** for implicit conversions. Each test file focuses on one specific context.

---

## Test Files

| File | Context Type | Purpose |
|------|-------------|---------|
| `assignment_context.c` | ASSIGNMENT_CONTEXT | Simple variable assignments |
| `loop_context.c` | LOOP_CONTEXT | Loop conditions (for, while, do-while) |
| `memory_context.c` | MEMORY_CONTEXT | Array indexing operations |
| `branch_context.c` | BRANCH_CONTEXT | If/else conditions, ternary operators |
| `api_context.c` | API_CONTEXT | Function call arguments |
| `return_context.c` | RETURN_CONTEXT | Return statements |

---

## How to Run

### Run All Tests

```bash
cd "/Users/shreyasgopinath/Desktop/CD Lab EL"
./run_validation_tests.sh
```

### Run Individual Tests

```bash
cd "/Users/shreyasgopinath/Desktop/CD Lab EL"

# Test assignment context
./build/src/icha tests/validation/assignment_context.c --

# Test loop context
./build/src/icha tests/validation/loop_context.c --

# Test memory context
./build/src/icha tests/validation/memory_context.c --

# Test branch context
./build/src/icha tests/validation/branch_context.c --

# Test API context
./build/src/icha tests/validation/api_context.c --

# Test return context
./build/src/icha tests/validation/return_context.c --
```

---

## Expected Results

### assignment_context.c
- **Expected Findings**: 5 conversions
- **Expected Context**: ASSIGNMENT_CONTEXT
- **Risk Level**: MEDIUM (50/100)

### loop_context.c
- **Expected Findings**: 5 conversions
- **Expected Context**: LOOP_CONTEXT
- **Risk Level**: MEDIUM to HIGH (context risk should be 25)

### memory_context.c
- **Expected Findings**: 5+ conversions
- **Expected Context**: MEMORY_CONTEXT or array indexing
- **Risk Level**: HIGH to CRITICAL (context risk should be 30)
- **Note**: This is the most dangerous context!

### branch_context.c
- **Expected Findings**: 5 conversions
- **Expected Context**: BRANCH_CONTEXT
- **Risk Level**: MEDIUM (context risk should be 20)

### api_context.c
- **Expected Findings**: 5 conversions
- **Expected Context**: API_CONTEXT or function call
- **Risk Level**: MEDIUM (context risk should be 10)

### return_context.c
- **Expected Findings**: 6 conversions
- **Expected Context**: RETURN_CONTEXT
- **Risk Level**: MEDIUM (context risk should be 15)

---

## Validation Criteria

For each test file, verify:

1. ✅ **Correct Context Detection**: ICHA identifies the right context type
2. ✅ **Appropriate Risk Score**: Context risk matches expected values
3. ✅ **All Conversions Found**: No conversions missed
4. ✅ **Correct Severity**: Severity matches risk level

---

## Context Risk Scores (Reference)

According to ICHA's risk model:

| Context | Risk Score | Priority |
|---------|-----------|----------|
| Array Indexing | 30 | Highest (memory safety) |
| Loop Condition | 25 | High (iteration bounds) |
| Branch Condition | 20 | Medium (control flow) |
| Return Statement | 15 | Medium (API contract) |
| API Boundary | 10 | Medium (function boundary) |
| Assignment | 5 | Low (local scope) |

---

## Example Output

When you run a test, you should see output like:

```
[MEDIUM] Implicit conversion detected
File:       tests/validation/loop_context.c:11
Conversion: double → int
Context:    LOOP_CONTEXT          ← Verify this matches!
Risk Score: 70/100

Risk Breakdown:
  Conversion Risk:  20
  Context Risk:     25              ← Should be 25 for loops
  Impact Risk:      20
  Propagation Risk: 5
```

---

## Troubleshooting

### No findings shown
- Make sure you're in the ICHA directory
- Try: `./build/src/icha --icha-min-severity=0 tests/validation/FILE.c --`

### Wrong context detected
- This indicates a bug in ICHA's context detection
- Check the source code in `src/context/`
- File an issue with details

### Compilation errors
- The test files are designed to compile without includes
- If you see errors, check the file syntax

---

## Adding New Tests

To add a new validation test:

1. Create a new `.c` file in `tests/validation/`
2. Add clear comments explaining expected behavior
3. Include multiple test cases for the context
4. Update this README with expected results
5. Add to `run_validation_tests.sh`

---

## Notes

- These tests use **simple, self-contained code** to avoid compilation issues
- Each test file is **independent** and can be run separately
- Tests are designed to be **deterministic** - same results every time
- Comments in each file explain what ICHA should detect

---

## Quick Verification

Run this to verify all tests work:

```bash
cd "/Users/shreyasgopinath/Desktop/CD Lab EL"
for file in tests/validation/*.c; do
    echo "Testing: $file"
    ./build/src/icha "$file" -- | grep -E "(Total Findings|Context)" | head -2
    echo "---"
done
```

Expected: Each file should show findings with the correct context type.
