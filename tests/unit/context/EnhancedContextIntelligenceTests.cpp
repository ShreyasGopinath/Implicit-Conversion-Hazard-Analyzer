#include <gtest/gtest.h>

#include "context/EnhancedContextIntelligenceEngine.hpp"

using namespace context;

namespace {

parsing::ImplicitCastEvent makeEvent(
    const std::string& sourceType,
    const std::string& targetType,
    bool inLoop = false,
    bool inBranch = false,
    bool inArray = false,
    bool inReturn = false,
    bool inAPI = false,
    bool inArithmetic = false,
    bool inFunctionArg = false,
    bool inMemAlloc = false) {

    parsing::ImplicitCastEvent event{};
    event.sourceTypeName = sourceType;
    event.targetTypeName = targetType;
    event.metadata.inLoopCondition = inLoop;
    event.metadata.inBranchCondition = inBranch;
    event.metadata.inArraySubscript = inArray;
    event.metadata.inReturnStatement = inReturn;
    event.metadata.inApiBoundary = inAPI;
    event.metadata.inArithmeticExpression = inArithmetic;
    event.metadata.inFunctionArgument = inFunctionArg;
    event.metadata.inMemoryAllocation = inMemAlloc;
    event.metadata.parentNodeKind = "BinaryOperator";
    return event;
}

}  // namespace

// ============================================================================
// Priority Resolution Tests
// ============================================================================

TEST(ContextPriorityResolverTests, MemoryContextHasHighestPriority) {
    EXPECT_GT(ContextPriorityResolver::getPriority(ContextType::MEMORY_CONTEXT),
              ContextPriorityResolver::getPriority(ContextType::LOOP_CONTEXT));
    EXPECT_GT(ContextPriorityResolver::getPriority(ContextType::MEMORY_CONTEXT),
              ContextPriorityResolver::getPriority(ContextType::BRANCH_CONTEXT));
    EXPECT_GT(ContextPriorityResolver::getPriority(ContextType::MEMORY_CONTEXT),
              ContextPriorityResolver::getPriority(ContextType::ASSIGNMENT_CONTEXT));
}

TEST(ContextPriorityResolverTests, LoopContextOverridesBranch) {
    EXPECT_GT(ContextPriorityResolver::getPriority(ContextType::LOOP_CONTEXT),
              ContextPriorityResolver::getPriority(ContextType::BRANCH_CONTEXT));
}

TEST(ContextPriorityResolverTests, ReturnContextOverridesAssignment) {
    EXPECT_GT(ContextPriorityResolver::getPriority(ContextType::RETURN_CONTEXT),
              ContextPriorityResolver::getPriority(ContextType::ASSIGNMENT_CONTEXT));
}

TEST(ContextPriorityResolverTests, ShouldOverrideCorrectly) {
    EXPECT_TRUE(ContextPriorityResolver::shouldOverride(
        ContextType::MEMORY_CONTEXT, ContextType::LOOP_CONTEXT));
    EXPECT_FALSE(ContextPriorityResolver::shouldOverride(
        ContextType::ASSIGNMENT_CONTEXT, ContextType::MEMORY_CONTEXT));
}

TEST(ContextPriorityResolverTests, ResolvePrimaryContextMemory) {
    std::vector<ContextType> contexts = {
        ContextType::ASSIGNMENT_CONTEXT,
        ContextType::MEMORY_CONTEXT,
        ContextType::ARITHMETIC_CONTEXT
    };
    EXPECT_EQ(ContextPriorityResolver::resolvePrimaryContext(contexts),
              ContextType::MEMORY_CONTEXT);
}

TEST(ContextPriorityResolverTests, ResolvePrimaryContextLoop) {
    std::vector<ContextType> contexts = {
        ContextType::BRANCH_CONTEXT,
        ContextType::LOOP_CONTEXT,
        ContextType::ARITHMETIC_CONTEXT
    };
    EXPECT_EQ(ContextPriorityResolver::resolvePrimaryContext(contexts),
              ContextType::LOOP_CONTEXT);
}

TEST(ContextPriorityResolverTests, ResolvePrimaryContextEmpty) {
    std::vector<ContextType> contexts;
    EXPECT_EQ(ContextPriorityResolver::resolvePrimaryContext(contexts),
              ContextType::UNKNOWN_CONTEXT);
}

// ============================================================================
// Context Classification Tests
// ============================================================================

TEST(EnhancedContextEngineTests, ClassifyMemoryContext_ArrayIndex) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "unsigned int", false, false, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::MEMORY_CONTEXT);
    EXPECT_GT(classification.confidenceScore, 0.9f);
    EXPECT_FALSE(classification.semanticExplanation.empty());
}

TEST(EnhancedContextEngineTests, ClassifyLoopContext) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("float", "int", true, false, false);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::LOOP_CONTEXT);
    EXPECT_GT(classification.confidenceScore, 0.9f);
}

TEST(EnhancedContextEngineTests, ClassifyBranchContext) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("double", "int", false, true, false);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::BRANCH_CONTEXT);
    EXPECT_GT(classification.confidenceScore, 0.9f);
}

TEST(EnhancedContextEngineTests, ClassifyAPIContext) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "long", false, false, false, false, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::API_CONTEXT);
}

TEST(EnhancedContextEngineTests, ClassifyReturnContext) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("double", "int", false, false, false, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::RETURN_CONTEXT);
}

TEST(EnhancedContextEngineTests, ClassifyArithmeticContext) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "float", false, false, false, false, false, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::ARITHMETIC_CONTEXT);
}

TEST(EnhancedContextEngineTests, ClassifyAssignmentContext) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "long", false, false, false, false, false, false);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::ASSIGNMENT_CONTEXT);
}

// ============================================================================
// Priority Override Tests
// ============================================================================

TEST(EnhancedContextEngineTests, MemoryOverridesLoop) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    // Both memory (array) and loop context present
    auto event = makeEvent("int", "unsigned int", true, false, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::MEMORY_CONTEXT);
    EXPECT_TRUE(std::find(classification.secondaryContexts.begin(),
                         classification.secondaryContexts.end(),
                         ContextType::LOOP_CONTEXT) != classification.secondaryContexts.end());
}

TEST(EnhancedContextEngineTests, MemoryOverridesBranch) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    // Both memory and branch context present
    auto event = makeEvent("int", "unsigned int", false, true, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::MEMORY_CONTEXT);
    EXPECT_TRUE(std::find(classification.secondaryContexts.begin(),
                         classification.secondaryContexts.end(),
                         ContextType::BRANCH_CONTEXT) != classification.secondaryContexts.end());
}

TEST(EnhancedContextEngineTests, LoopOverridesBranch) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    // Both loop and branch context present
    auto event = makeEvent("float", "int", true, true, false);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::LOOP_CONTEXT);
    EXPECT_TRUE(std::find(classification.secondaryContexts.begin(),
                         classification.secondaryContexts.end(),
                         ContextType::BRANCH_CONTEXT) != classification.secondaryContexts.end());
}

TEST(EnhancedContextEngineTests, ReturnOverridesAssignment) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    // Return context (assignment is implicit)
    auto event = makeEvent("double", "int", false, false, false, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::RETURN_CONTEXT);
}

// ============================================================================
// Multiple Context Tests
// ============================================================================

TEST(EnhancedContextEngineTests, MultipleContexts_MemoryLoopBranch) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    // Memory, loop, and branch all present
    auto event = makeEvent("int", "unsigned int", true, true, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::MEMORY_CONTEXT);
    EXPECT_GE(classification.secondaryContexts.size(), 2u);
}

TEST(EnhancedContextEngineTests, MultipleContexts_APIAndArithmetic) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    // API and arithmetic context
    auto event = makeEvent("int", "long", false, false, false, false, true, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::API_CONTEXT);
    EXPECT_TRUE(std::find(classification.secondaryContexts.begin(),
                         classification.secondaryContexts.end(),
                         ContextType::ARITHMETIC_CONTEXT) != classification.secondaryContexts.end());
}

// ============================================================================
// Confidence Scoring Tests
// ============================================================================

TEST(EnhancedContextEngineTests, HighConfidence_SingleContext) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "unsigned int", false, false, true);

    auto classification = engine->classifyContext(event);

    EXPECT_GT(classification.confidenceScore, 0.9f);
}

TEST(EnhancedContextEngineTests, MediumConfidence_TwoContexts) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "unsigned int", true, false, true);

    auto classification = engine->classifyContext(event);

    EXPECT_GT(classification.confidenceScore, 0.7f);
    EXPECT_LT(classification.confidenceScore, 0.9f);
}

TEST(EnhancedContextEngineTests, LowerConfidence_MultipleContexts) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "unsigned int", true, true, true);

    auto classification = engine->classifyContext(event);

    EXPECT_GT(classification.confidenceScore, 0.5f);
    EXPECT_LT(classification.confidenceScore, 0.8f);
}

// ============================================================================
// Explanation Generation Tests
// ============================================================================

TEST(EnhancedContextEngineTests, ExplanationContainsContextType) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "unsigned int", false, false, true);

    auto classification = engine->classifyContext(event);

    EXPECT_NE(classification.semanticExplanation.find("MEMORY_CONTEXT"), std::string::npos);
    EXPECT_NE(classification.semanticExplanation.find("memory access"), std::string::npos);
}

TEST(EnhancedContextEngineTests, ExplanationContainsConfidence) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("float", "int", true);

    auto classification = engine->classifyContext(event);

    EXPECT_NE(classification.semanticExplanation.find("Confidence"), std::string::npos);
}

TEST(EnhancedContextEngineTests, ExplanationContainsSecondaryContexts) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "unsigned int", true, false, true);

    auto classification = engine->classifyContext(event);

    if (!classification.secondaryContexts.empty()) {
        EXPECT_NE(classification.semanticExplanation.find("Secondary contexts"), std::string::npos);
    }
}

// ============================================================================
// AST Context Path Tests
// ============================================================================

TEST(EnhancedContextEngineTests, ASTPathContainsImplicitCast) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "long");

    auto path = engine->getASTContextPath(event);

    EXPECT_FALSE(path.empty());
    EXPECT_EQ(path[0], "ImplicitCastExpr");
}

TEST(EnhancedContextEngineTests, ASTPathContainsArraySubscript) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "unsigned int", false, false, true);

    auto path = engine->getASTContextPath(event);

    EXPECT_TRUE(std::find(path.begin(), path.end(), "ArraySubscriptExpr") != path.end());
}

TEST(EnhancedContextEngineTests, ASTPathContainsLoopStmt) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("float", "int", true);

    auto path = engine->getASTContextPath(event);

    EXPECT_TRUE(std::find(path.begin(), path.end(), "LoopStmt") != path.end());
}

// ============================================================================
// Integration Tests with Callback
// ============================================================================

TEST(EnhancedContextEngineTests, AnalyzeWithCallback) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "unsigned int", false, false, true);

    bool callbackInvoked = false;
    EnhancedContextRecord receivedRecord;

    engine->analyze(event, [&](const EnhancedContextRecord& record) {
        callbackInvoked = true;
        receivedRecord = record;
    });

    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(receivedRecord.classification.primaryContext, ContextType::MEMORY_CONTEXT);
}

TEST(EnhancedContextEngineTests, LegacyFlagsPopulated) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    auto event = makeEvent("int", "unsigned int", false, false, true);

    EnhancedContextRecord receivedRecord;
    engine->analyze(event, [&](const EnhancedContextRecord& record) {
        receivedRecord = record;
    });

    EXPECT_TRUE(receivedRecord.inArrayIndex);
    EXPECT_FALSE(receivedRecord.inLoopCondition);
}

// ============================================================================
// Real-World Scenario Tests
// ============================================================================

TEST(EnhancedContextEngineTests, Scenario_SignedToUnsignedArrayIndex) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    // int n = -1; unsigned idx = n; arr[idx];
    auto event = makeEvent("int", "unsigned int", false, false, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::MEMORY_CONTEXT);
    EXPECT_GT(classification.confidenceScore, 0.9f);
    EXPECT_NE(classification.semanticExplanation.find("array indexing"), std::string::npos);
}

TEST(EnhancedContextEngineTests, Scenario_FloatToIntLoopBound) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    // float f = 10.9; int limit = f; for(i=0; i<limit; i++)
    auto event = makeEvent("float", "int", true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::LOOP_CONTEXT);
    EXPECT_NE(classification.semanticExplanation.find("iteration"), std::string::npos);
}

TEST(EnhancedContextEngineTests, Scenario_DoubleToIntReturn) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    // double calculate() { return 3.14; } -> int x = calculate();
    auto event = makeEvent("double", "int", false, false, false, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::RETURN_CONTEXT);
    EXPECT_NE(classification.semanticExplanation.find("function contract"), std::string::npos);
}

TEST(EnhancedContextEngineTests, Scenario_ComplexMultiContext) {
    auto engine = makeEnhancedContextIntelligenceEngine();
    // Complex: for(int i=0; i<limit; i++) { if(condition) arr[convert(i)] = value; }
    auto event = makeEvent("long", "int", true, true, true);

    auto classification = engine->classifyContext(event);

    EXPECT_EQ(classification.primaryContext, ContextType::MEMORY_CONTEXT);
    EXPECT_GE(classification.secondaryContexts.size(), 1u);
    EXPECT_LT(classification.confidenceScore, 0.8f);  // Lower confidence due to complexity
}
