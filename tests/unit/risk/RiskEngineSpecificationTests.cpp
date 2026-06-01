#include <gtest/gtest.h>

#include "context/ContextRecord.hpp"
#include "domain/conversion/ConversionRecord.hpp"
#include "risk/RiskEngine.hpp"

namespace {

// Helper to create conversion records
domain::conversion::ConversionRecord makeConversion(
    const std::string& source,
    const std::string& target,
    bool sourceSigned = false,
    bool targetSigned = false,
    bool sourceEnum = false,
    bool targetEnum = false,
    bool sourcePointer = false,
    bool targetPointer = false) {
    domain::conversion::ConversionRecord record{};
    record.sourceType = source;
    record.targetType = target;
    record.sourceIsSigned = sourceSigned;
    record.targetIsSigned = targetSigned;
    record.sourceIsEnum = sourceEnum;
    record.targetIsEnum = targetEnum;
    record.sourceIsPointer = sourcePointer;
    record.targetIsPointer = targetPointer;
    return record;
}

// Helper to create context records
context::ContextRecord makeContext(
    bool arrayIndex = false,
    bool loopCondition = false,
    bool branchCondition = false,
    bool functionArg = false,
    bool returnStmt = false,
    bool arithmetic = false,
    bool assignmentOnly = false) {
    context::ContextRecord ctx{};
    ctx.inArrayIndex = arrayIndex;
    ctx.inLoopCondition = loopCondition;
    ctx.inBranchCondition = branchCondition;
    ctx.inFunctionArgument = functionArg;
    ctx.inReturnStatement = returnStmt;
    ctx.inArithmeticExpression = arithmetic;
    ctx.isAssignmentOnly = assignmentOnly;
    return ctx;
}

// Helper to create impact profiles
risk::ImpactProfile makeImpact(
    bool memoryAccess = false,
    bool controlFlow = false,
    bool arithmetic = false) {
    risk::ImpactProfile profile{};
    profile.affectsMemoryAccess = memoryAccess;
    profile.affectsControlFlow = controlFlow;
    profile.affectsArithmetic = arithmetic;
    return profile;
}

}  // namespace

// ============================================================================
// CONVERSION RISK TESTS (CR)
// ============================================================================

TEST(RiskEngineSpecificationTests, ConversionRisk_DoubleToInt_Equals25) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 25u);
}

TEST(RiskEngineSpecificationTests, ConversionRisk_FloatToInt_Equals20) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("float", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 20u);
}

TEST(RiskEngineSpecificationTests, ConversionRisk_LongToInt_Equals15) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("long", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 15u);
}

TEST(RiskEngineSpecificationTests, ConversionRisk_IntToShort_Equals15) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("int", "short"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 15u);
}

TEST(RiskEngineSpecificationTests, ConversionRisk_SignedToUnsigned_Equals30) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("int", "unsigned int", true, false),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 30u);
}

TEST(RiskEngineSpecificationTests, ConversionRisk_UnsignedToSigned_Equals25) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("unsigned int", "int", false, true),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 25u);
}

TEST(RiskEngineSpecificationTests, ConversionRisk_EnumToInt_Equals10) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("enum Color", "int", false, false, true, false),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 10u);
}

TEST(RiskEngineSpecificationTests, ConversionRisk_IntToEnum_Equals15) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("int", "enum Status", false, false, false, true),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 15u);
}

TEST(RiskEngineSpecificationTests, ConversionRisk_PointerToInteger_Equals35) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("int *", "long", false, false, false, false, true, false),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 35u);
}

// ============================================================================
// CONTEXT RISK TESTS (CTX)
// ============================================================================

TEST(RiskEngineSpecificationTests, ContextRisk_ArrayIndexing_Equals30) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(true, false, false, false, false, false, false),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.contextRisk, 30u);
}

TEST(RiskEngineSpecificationTests, ContextRisk_LoopCondition_Equals25) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(false, true, false, false, false, false, false),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.contextRisk, 25u);
}

TEST(RiskEngineSpecificationTests, ContextRisk_BranchCondition_Equals15) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(false, false, true, false, false, false, false),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.contextRisk, 15u);
}

TEST(RiskEngineSpecificationTests, ContextRisk_FunctionArgument_Equals10) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(false, false, false, true, false, false, false),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.contextRisk, 10u);
}

TEST(RiskEngineSpecificationTests, ContextRisk_ReturnStatement_Equals15) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(false, false, false, false, true, false, false),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.contextRisk, 15u);
}

TEST(RiskEngineSpecificationTests, ContextRisk_ArithmeticExpression_Equals10) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(false, false, false, false, false, true, false),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.contextRisk, 10u);
}

TEST(RiskEngineSpecificationTests, ContextRisk_AssignmentOnly_Equals5) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(false, false, false, false, false, false, true),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.contextRisk, 5u);
}

// ============================================================================
// IMPACT RISK TESTS (IR)
// ============================================================================

TEST(RiskEngineSpecificationTests, ImpactRisk_MemoryAccess_Equals30) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(true, false, false),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.impactRisk, 30u);
}

TEST(RiskEngineSpecificationTests, ImpactRisk_ControlFlow_Equals20) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(false, true, false),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.impactRisk, 20u);
}

TEST(RiskEngineSpecificationTests, ImpactRisk_Arithmetic_Equals10) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(false, false, true),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.impactRisk, 10u);
}

TEST(RiskEngineSpecificationTests, ImpactRisk_NoEffect_Equals0) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(false, false, false),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.impactRisk, 0u);
}

// ============================================================================
// PROPAGATION RISK TESTS (PR)
// ============================================================================

TEST(RiskEngineSpecificationTests, PropagationRisk_ImmediateUse_Equals5) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::ImmediateUse);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 5u);
}

TEST(RiskEngineSpecificationTests, PropagationRisk_SameExpressionChain_Equals10) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::SameExpressionChain);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 10u);
}

TEST(RiskEngineSpecificationTests, PropagationRisk_CrossFunction_Equals20) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::CrossFunction);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 20u);
}

TEST(RiskEngineSpecificationTests, PropagationRisk_ModuleBoundary_Equals25) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::ModuleBoundary);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 25u);
}

TEST(RiskEngineSpecificationTests, PropagationRisk_ApiBoundary_Equals30) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::ApiBoundary);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 30u);
}

TEST(RiskEngineSpecificationTests, PropagationRisk_None_Equals0) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 0u);
}

// ============================================================================
// SEVERITY MAPPING TESTS
// ============================================================================

TEST(RiskEngineSpecificationTests, Severity_Score0_IsLow) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("int", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.total(), 0u);
    EXPECT_EQ(assessment.severity, risk::Severity::Low);
}

TEST(RiskEngineSpecificationTests, Severity_Score20_IsLow) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("float", "int"),  // CR=20
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.total(), 20u);
    EXPECT_EQ(assessment.severity, risk::Severity::Low);
}

TEST(RiskEngineSpecificationTests, Severity_Score21_IsMedium) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("float", "int"),  // CR=20
        makeContext(false, false, false, false, false, false, true),  // CTX=5
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.total(), 25u);
    EXPECT_EQ(assessment.severity, risk::Severity::Medium);
}

TEST(RiskEngineSpecificationTests, Severity_Score40_IsMedium) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),  // CR=25
        makeContext(false, false, true, false, false, false, false),  // CTX=15
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.total(), 40u);
    EXPECT_EQ(assessment.severity, risk::Severity::Medium);
}

TEST(RiskEngineSpecificationTests, Severity_Score41_IsHigh) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),  // CR=25
        makeContext(false, false, false, false, true, false, false),  // CTX=15
        makeImpact(),
        risk::PropagationLevel::ImmediateUse);  // PR=5
    EXPECT_EQ(assessment.breakdown.total(), 45u);
    EXPECT_EQ(assessment.severity, risk::Severity::High);
}

TEST(RiskEngineSpecificationTests, Severity_Score70_IsHigh) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),  // CR=25
        makeContext(false, true, false, false, false, false, false),  // CTX=25
        makeImpact(false, true, false),  // IR=20
        risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.total(), 70u);
    EXPECT_EQ(assessment.severity, risk::Severity::High);
}

TEST(RiskEngineSpecificationTests, Severity_Score71_IsCritical) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),  // CR=25
        makeContext(false, true, false, false, false, false, false),  // CTX=25
        makeImpact(false, true, false),  // IR=20
        risk::PropagationLevel::ImmediateUse);  // PR=5
    EXPECT_EQ(assessment.breakdown.total(), 75u);
    EXPECT_EQ(assessment.severity, risk::Severity::Critical);
}

// ============================================================================
// COMPREHENSIVE SCENARIO TESTS
// ============================================================================

TEST(RiskEngineSpecificationTests, Scenario_SignedToUnsignedArrayIndexing_Critical) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("int", "unsigned int", true, false),  // CR=30
        makeContext(true, false, false, false, false, false, false),  // CTX=30 (array)
        makeImpact(true, false, false),  // IR=30 (memory)
        risk::PropagationLevel::ImmediateUse);  // PR=5
    EXPECT_EQ(assessment.breakdown.conversionRisk, 30u);
    EXPECT_EQ(assessment.breakdown.contextRisk, 30u);
    EXPECT_EQ(assessment.breakdown.impactRisk, 30u);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 5u);
    EXPECT_EQ(assessment.breakdown.total(), 95u);
    EXPECT_EQ(assessment.severity, risk::Severity::Critical);
    EXPECT_FALSE(assessment.explanation.empty());
    EXPECT_FALSE(assessment.suggestedFix.empty());
}

TEST(RiskEngineSpecificationTests, Scenario_PointerToIntegerApiBoundary_Critical) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("void *", "long", false, false, false, false, true, false),  // CR=35
        makeContext(false, false, false, true, false, false, false),  // CTX=10 (function arg)
        makeImpact(false, false, true),  // IR=10 (arithmetic)
        risk::PropagationLevel::ApiBoundary);  // PR=30
    EXPECT_EQ(assessment.breakdown.conversionRisk, 35u);
    EXPECT_EQ(assessment.breakdown.contextRisk, 10u);
    EXPECT_EQ(assessment.breakdown.impactRisk, 10u);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 30u);
    EXPECT_EQ(assessment.breakdown.total(), 85u);
    EXPECT_EQ(assessment.severity, risk::Severity::Critical);
}

TEST(RiskEngineSpecificationTests, Scenario_FloatToIntLoopCondition_High) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("float", "int"),  // CR=20
        makeContext(false, true, false, false, false, false, false),  // CTX=25 (loop)
        makeImpact(false, true, false),  // IR=20 (control flow)
        risk::PropagationLevel::None);  // PR=0
    EXPECT_EQ(assessment.breakdown.conversionRisk, 20u);
    EXPECT_EQ(assessment.breakdown.contextRisk, 25u);
    EXPECT_EQ(assessment.breakdown.impactRisk, 20u);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 0u);
    EXPECT_EQ(assessment.breakdown.total(), 65u);
    EXPECT_EQ(assessment.severity, risk::Severity::High);
}

TEST(RiskEngineSpecificationTests, Scenario_EnumToIntAssignment_Low) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("enum Color", "int", false, false, true, false),  // CR=10
        makeContext(false, false, false, false, false, false, true),  // CTX=5 (assignment)
        makeImpact(false, false, false),  // IR=0
        risk::PropagationLevel::None);  // PR=0
    EXPECT_EQ(assessment.breakdown.conversionRisk, 10u);
    EXPECT_EQ(assessment.breakdown.contextRisk, 5u);
    EXPECT_EQ(assessment.breakdown.impactRisk, 0u);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 0u);
    EXPECT_EQ(assessment.breakdown.total(), 15u);
    EXPECT_EQ(assessment.severity, risk::Severity::Low);
}

// ============================================================================
// OUTPUT VALIDATION TESTS
// ============================================================================

TEST(RiskEngineSpecificationTests, Output_HasExplanation) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(true, false, false, false, false, false, false),
        makeImpact(true, false, false),
        risk::PropagationLevel::ApiBoundary);
    EXPECT_FALSE(assessment.explanation.empty());
    EXPECT_NE(assessment.explanation.find("double"), std::string::npos);
    EXPECT_NE(assessment.explanation.find("int"), std::string::npos);
}

TEST(RiskEngineSpecificationTests, Output_HasSuggestedFix) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_FALSE(assessment.suggestedFix.empty());
    EXPECT_NE(assessment.suggestedFix.find("static_cast"), std::string::npos);
}

TEST(RiskEngineSpecificationTests, Output_HasJsonFormat) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(
        makeConversion("double", "int"),
        makeContext(),
        makeImpact(),
        risk::PropagationLevel::None);
    EXPECT_FALSE(assessment.json.empty());
    EXPECT_NE(assessment.json.find("conversion"), std::string::npos);
    EXPECT_NE(assessment.json.find("risk_breakdown"), std::string::npos);
}
