#include <gtest/gtest.h>

#include "context/ContextIntelligenceEngine.hpp"
#include "parsing/ImplicitCastEvent.hpp"

namespace {
parsing::ImplicitCastEvent makeEvent(const std::string& parentKind) {
    parsing::ImplicitCastEvent event{};
    event.metadata.parentNodeKind = parentKind;
    if (parentKind == "IfStmt") {
        event.metadata.inBranchCondition = true;
    }
    if (parentKind == "ForStmt") {
        event.metadata.inLoopCondition = true;
    }
    if (parentKind == "ArraySubscriptExpr") {
        event.metadata.inArraySubscript = true;
    }
    if (parentKind == "ReturnStmt") {
        event.metadata.inReturnStatement = true;
    }
    if (parentKind == "CallExpr") {
        event.metadata.inApiBoundary = true;
    }
    if (parentKind == "BinaryOperator") {
        event.metadata.inArithmeticExpression = true;
    }
    return event;
}
}

TEST(ContextIntelligenceTests, DetectsBranchConditions) {
    auto engine = context::makeContextIntelligenceEngine();
    const auto event = makeEvent("IfStmt");
    context::ContextRecord record;
    engine->analyze(event, [&](const context::ContextRecord& r) { record = r; });
    EXPECT_TRUE(record.inBranchCondition);
}

TEST(ContextIntelligenceTests, DetectsLoopConditions) {
    auto engine = context::makeContextIntelligenceEngine();
    const auto event = makeEvent("ForStmt");
    context::ContextRecord record;
    engine->analyze(event, [&](const context::ContextRecord& r) { record = r; });
    EXPECT_TRUE(record.inLoopCondition);
}

TEST(ContextIntelligenceTests, DetectsArrayIndex) {
    auto engine = context::makeContextIntelligenceEngine();
    const auto event = makeEvent("ArraySubscriptExpr");
    context::ContextRecord record;
    engine->analyze(event, [&](const context::ContextRecord& r) { record = r; });
    EXPECT_TRUE(record.inArrayIndex);
}

TEST(ContextIntelligenceTests, DetectsApiBoundary) {
    auto engine = context::makeContextIntelligenceEngine();
    auto event = makeEvent("CallExpr");
    context::ContextRecord record;
    engine->analyze(event, [&](const context::ContextRecord& r) { record = r; });
    EXPECT_TRUE(record.inApiBoundary);
}

TEST(ContextIntelligenceTests, DetectsArithmetic) {
    auto engine = context::makeContextIntelligenceEngine();
    auto event = makeEvent("BinaryOperator");
    context::ContextRecord record;
    engine->analyze(event, [&](const context::ContextRecord& r) { record = r; });
    EXPECT_TRUE(record.inArithmeticExpression);
}
