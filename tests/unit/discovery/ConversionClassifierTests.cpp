#include <gtest/gtest.h>

#include "discovery/ConversionClassifier.hpp"
#include "parsing/ImplicitCastEvent.hpp"

namespace {
parsing::ImplicitCastEvent makeEvent(std::string source, std::string target) {
    parsing::ImplicitCastEvent event{};
    event.sourceTypeName = std::move(source);
    event.targetTypeName = std::move(target);
    return event;
}
}

TEST(ConversionClassifierTests, DetectsBooleanConversion) {
    auto classifier = discovery::makeDefaultConversionClassifier();
    const auto record = classifier->classify(makeEvent("int", "bool"));
    EXPECT_EQ(record.category, domain::conversion::ConversionCategory::BooleanConversion);
}

TEST(ConversionClassifierTests, DetectsPointerConversion) {
    auto classifier = discovery::makeDefaultConversionClassifier();
    const auto record = classifier->classify(makeEvent("int*", "void*"));
    EXPECT_EQ(record.category, domain::conversion::ConversionCategory::PointerConversion);
}

TEST(ConversionClassifierTests, DetectsSignChange) {
    auto classifier = discovery::makeDefaultConversionClassifier();
    const auto record = classifier->classify(makeEvent("unsigned int", "int"));
    EXPECT_EQ(record.category, domain::conversion::ConversionCategory::SignChange);
}

TEST(ConversionClassifierTests, DetectsNarrowing) {
    auto classifier = discovery::makeDefaultConversionClassifier();
    const auto record = classifier->classify(makeEvent("double", "float"));
    EXPECT_EQ(record.category, domain::conversion::ConversionCategory::Narrowing);
}

TEST(ConversionClassifierTests, DetectsWidening) {
    auto classifier = discovery::makeDefaultConversionClassifier();
    const auto record = classifier->classify(makeEvent("int", "long"));
    EXPECT_EQ(record.category, domain::conversion::ConversionCategory::Widening);
}
