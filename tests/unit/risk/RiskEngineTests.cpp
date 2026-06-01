#include <gtest/gtest.h>

#include "context/ContextRecord.hpp"
#include "domain/conversion/ConversionRecord.hpp"
#include "risk/RiskEngine.hpp"

namespace {
domain::conversion::ConversionRecord makeConversion(const std::string& src,
                                                     const std::string& dst,
                                                     bool sourceSigned = true,
                                                     bool targetSigned = true) {
    domain::conversion::ConversionRecord record{};
    record.sourceType = src;
    record.targetType = dst;
    record.sourceIsSigned = sourceSigned;
    record.targetIsSigned = targetSigned;
    return record;
}

context::ContextRecord makeContext(bool loop = false, bool branch = false, bool array = false,
                                   bool api = false) {
    context::ContextRecord ctx{};
    ctx.inLoopCondition = loop;
    ctx.inBranchCondition = branch;
    ctx.inArrayIndex = array;
    ctx.inApiBoundary = api;
    return ctx;
}

risk::ImpactProfile memoryImpact() {
    risk::ImpactProfile profile{};
    profile.affectsMemoryAccess = true;
    return profile;
}

risk::ImpactProfile controlImpact() {
    risk::ImpactProfile profile{};
    profile.affectsControlFlow = true;
    return profile;
}

}  // namespace

TEST(RiskEngineTests, DoubleToIntLoopAndMemory) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(makeConversion("double", "int"),
                                     makeContext(true, false, false), memoryImpact(),
                                     risk::PropagationLevel::ImmediateUse);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 25u);
    EXPECT_EQ(assessment.breakdown.contextRisk, 25u);
    EXPECT_EQ(assessment.breakdown.impactRisk, 30u);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 5u);
    EXPECT_EQ(assessment.breakdown.total(), 85u);
    EXPECT_EQ(assessment.severity, risk::Severity::Critical);
}

TEST(RiskEngineTests, FloatToIntBranchControlCrossFunction) {
    auto engine = risk::makeRiskEngine();
    auto assessment = engine->assess(makeConversion("float", "int"),
                                     makeContext(false, true, false), controlImpact(),
                                     risk::PropagationLevel::CrossFunction);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 20u);
    EXPECT_EQ(assessment.breakdown.contextRisk, 15u);
    EXPECT_EQ(assessment.breakdown.impactRisk, 20u);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 20u);
    EXPECT_EQ(assessment.breakdown.total(), 75u);
    EXPECT_EQ(assessment.severity, risk::Severity::Critical);
}

TEST(RiskEngineTests, SignedToUnsignedArrayApiBoundary) {
    auto engine = risk::makeRiskEngine();
    domain::conversion::ConversionRecord conversion =
        makeConversion("int", "unsigned int", true, false);

    context::ContextRecord ctx = makeContext(false, false, true, true);

    risk::ImpactProfile impact{};
    risk::RiskAssessment assessment = engine->assess(conversion, ctx, impact,
                                                     risk::PropagationLevel::ApiBoundary);
    EXPECT_EQ(assessment.breakdown.conversionRisk, 30u);
    EXPECT_EQ(assessment.breakdown.contextRisk, 30u);
    EXPECT_EQ(assessment.breakdown.impactRisk, 0u);
    EXPECT_EQ(assessment.breakdown.propagationRisk, 30u);
    EXPECT_EQ(assessment.breakdown.total(), 90u);
    EXPECT_EQ(assessment.severity, risk::Severity::Critical);
}

TEST(RiskEngineTests, NoRiskScenario) {
    auto engine = risk::makeRiskEngine();
    domain::conversion::ConversionRecord conversion = makeConversion("int", "int");
    context::ContextRecord ctx = makeContext();
    risk::ImpactProfile impact{};
    auto assessment = engine->assess(conversion, ctx, impact, risk::PropagationLevel::None);
    EXPECT_EQ(assessment.breakdown.total(), 0u);
    EXPECT_EQ(assessment.severity, risk::Severity::Low);
}

