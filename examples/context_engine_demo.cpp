/**
 * Enhanced Context Intelligence Engine Demonstration
 *
 * Demonstrates sophisticated context classification with priority rules,
 * confidence scoring, and detailed explanations.
 */

#include <iostream>
#include <iomanip>

#include "context/EnhancedContextIntelligenceEngine.hpp"

using namespace context;

void printSeparator(char c = '=') {
    std::cout << std::string(80, c) << '\n';
}

void printContextType(ContextType type) {
    std::cout << contextTypeToString(type);
}

void demonstrateContextClassification(
    const std::string& scenario,
    const std::string& codeSnippet,
    const parsing::ImplicitCastEvent& event) {

    printSeparator();
    std::cout << "SCENARIO: " << scenario << '\n';
    printSeparator();

    std::cout << "\nCode Snippet:\n";
    std::cout << "  " << codeSnippet << '\n';

    std::cout << "\nConversion: " << event.sourceTypeName
              << " -> " << event.targetTypeName << '\n';

    auto engine = makeEnhancedContextIntelligenceEngine();
    auto classification = engine->classifyContext(event);

    std::cout << "\n[CONTEXT CLASSIFICATION]\n";
    std::cout << "  Primary Context: ";
    printContextType(classification.primaryContext);
    std::cout << '\n';

    if (!classification.secondaryContexts.empty()) {
        std::cout << "  Secondary Contexts: ";
        for (size_t i = 0; i < classification.secondaryContexts.size(); ++i) {
            if (i > 0) std::cout << ", ";
            printContextType(classification.secondaryContexts[i]);
        }
        std::cout << '\n';
    }

    std::cout << "  Confidence Score: "
              << std::fixed << std::setprecision(1)
              << (classification.confidenceScore * 100) << "%\n";

    std::cout << "\n[AST CONTEXT PATH]\n";
    for (size_t i = 0; i < classification.astContextPath.size(); ++i) {
        std::cout << "  ";
        for (size_t j = 0; j < i; ++j) std::cout << "  ";
        std::cout << "└─ " << classification.astContextPath[i] << '\n';
    }

    std::cout << "\n[SEMANTIC EXPLANATION]\n";
    std::cout << "  " << classification.semanticExplanation << '\n';

    std::cout << '\n';
}

parsing::ImplicitCastEvent makeEvent(
    const std::string& source,
    const std::string& target,
    bool inLoop = false,
    bool inBranch = false,
    bool inArray = false,
    bool inReturn = false,
    bool inAPI = false,
    bool inArithmetic = false,
    bool inMemAlloc = false) {

    parsing::ImplicitCastEvent event{};
    event.sourceTypeName = source;
    event.targetTypeName = target;
    event.metadata.inLoopCondition = inLoop;
    event.metadata.inBranchCondition = inBranch;
    event.metadata.inArraySubscript = inArray;
    event.metadata.inReturnStatement = inReturn;
    event.metadata.inApiBoundary = inAPI;
    event.metadata.inArithmeticExpression = inArithmetic;
    event.metadata.inMemoryAllocation = inMemAlloc;
    event.metadata.parentNodeKind = "BinaryOperator";
    return event;
}

int main() {
    std::cout << "\n";
    printSeparator();
    std::cout << "  ENHANCED CONTEXT INTELLIGENCE ENGINE - DEMONSTRATION\n";
    printSeparator();
    std::cout << "\n";

    // ========================================================================
    // Scenario 1: Memory Context (Highest Priority)
    // ========================================================================
    demonstrateContextClassification(
        "Memory Context - Array Indexing",
        "int n = -1; unsigned idx = n; arr[idx];",
        makeEvent("int", "unsigned int", false, false, true)
    );

    // ========================================================================
    // Scenario 2: Loop Context
    // ========================================================================
    demonstrateContextClassification(
        "Loop Context - Iteration Bound",
        "float f = 10.9; int limit = f; for(i=0; i<limit; i++)",
        makeEvent("float", "int", true, false, false)
    );

    // ========================================================================
    // Scenario 3: Branch Context
    // ========================================================================
    demonstrateContextClassification(
        "Branch Context - Conditional Logic",
        "double d = 3.14; int x = d; if(x > 0) {...}",
        makeEvent("double", "int", false, true, false)
    );

    // ========================================================================
    // Scenario 4: API Context
    // ========================================================================
    demonstrateContextClassification(
        "API Context - Function Boundary",
        "int value = -1; processData((unsigned)value);",
        makeEvent("int", "unsigned int", false, false, false, false, true)
    );

    // ========================================================================
    // Scenario 5: Return Context
    // ========================================================================
    demonstrateContextClassification(
        "Return Context - Function Contract",
        "double calculate() { return 3.14159; } int x = calculate();",
        makeEvent("double", "int", false, false, false, true)
    );

    // ========================================================================
    // Scenario 6: Priority Override - Memory > Loop
    // ========================================================================
    demonstrateContextClassification(
        "Priority Override - Memory Overrides Loop",
        "for(int i=0; i<n; i++) { arr[(unsigned)i] = value; }",
        makeEvent("int", "unsigned int", true, false, true)
    );

    // ========================================================================
    // Scenario 7: Priority Override - Memory > Branch
    // ========================================================================
    demonstrateContextClassification(
        "Priority Override - Memory Overrides Branch",
        "if(condition) { arr[(unsigned)index] = value; }",
        makeEvent("int", "unsigned int", false, true, true)
    );

    // ========================================================================
    // Scenario 8: Priority Override - Loop > Branch
    // ========================================================================
    demonstrateContextClassification(
        "Priority Override - Loop Overrides Branch",
        "for(int i=0; i<limit; i++) { if((bool)i) {...} }",
        makeEvent("int", "bool", true, true, false)
    );

    // ========================================================================
    // Scenario 9: Complex Multi-Context
    // ========================================================================
    demonstrateContextClassification(
        "Complex Multi-Context Scenario",
        "for(...) { if(...) { arr[convert(x)] = api_call(y); } }",
        makeEvent("long", "int", true, true, true, false, true)
    );

    // ========================================================================
    // Scenario 10: Simple Assignment (Lowest Priority)
    // ========================================================================
    demonstrateContextClassification(
        "Assignment Context - Simple Assignment",
        "int x = 10; long y = x;",
        makeEvent("int", "long", false, false, false, false, false, false)
    );

    printSeparator();
    std::cout << "\nKEY FEATURES DEMONSTRATED:\n";
    std::cout << "  1. Priority-based context resolution\n";
    std::cout << "  2. Multi-context detection with primary/secondary classification\n";
    std::cout << "  3. Confidence scoring based on context complexity\n";
    std::cout << "  4. AST context path extraction\n";
    std::cout << "  5. Detailed semantic explanations\n";
    std::cout << "\nPRIORITY RULES:\n";
    std::cout << "  MEMORY_CONTEXT (100) > LOOP_CONTEXT (80) > API_CONTEXT (70)\n";
    std::cout << "  > RETURN_CONTEXT (60) > BRANCH_CONTEXT (50) > ARITHMETIC_CONTEXT (30)\n";
    std::cout << "  > ASSIGNMENT_CONTEXT (20)\n";
    printSeparator();
    std::cout << "\n";

    return 0;
}
