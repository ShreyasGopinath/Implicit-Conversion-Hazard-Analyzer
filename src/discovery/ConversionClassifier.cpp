#include "discovery/ConversionClassifier.hpp"

#include <utility>

namespace discovery {
namespace {
bool isBooleanType(const std::string& typeName) {
    return typeName == "bool" || typeName == "_Bool";
}

bool isUnsignedType(const std::string& typeName) {
    return typeName.find("unsigned") != std::string::npos;
}

bool isSignedType(const std::string& typeName) {
    if (isUnsignedType(typeName)) {
        return false;
    }
    return typeName.find("signed") != std::string::npos || (typeName == "int" || typeName == "long" ||
                                                            typeName == "short" || typeName == "long long");
}

bool isFloatingType(const std::string& typeName) {
    return typeName == "float" || typeName == "double" || typeName == "long double";
}

bool isPointerType(const std::string& typeName) {
    return typeName.find('*') != std::string::npos;
}

bool isEnumType(const std::string& typeName) {
    return typeName.find("enum ") == 0;
}

bool isNarrowing(const std::string& source, const std::string& target) {
    if (isFloatingType(source) && !isFloatingType(target)) {
        return true;
    }
    if (source == "long double" && target == "double") {
        return true;
    }
    if (source == "double" && target == "float") {
        return true;
    }
    if ((source == "long" || source == "long long") && (target == "int" || target == "short")) {
        return true;
    }
    return false;
}

bool isWidening(const std::string& source, const std::string& target) {
    if (!isFloatingType(source) && isFloatingType(target)) {
        return true;
    }
    if ((source == "int" && target == "long") || (source == "short" && target == "int")) {
        return true;
    }
    return false;
}

class DefaultConversionClassifier : public ConversionClassifier {
   public:
    domain::conversion::ConversionRecord classify(const parsing::ImplicitCastEvent& event) const override {
        domain::conversion::ConversionRecord record{};
        record.sourceType = event.sourceTypeName;
        record.targetType = event.targetTypeName;
        record.sourceIsBoolean = isBooleanType(event.sourceTypeName);
        record.targetIsBoolean = isBooleanType(event.targetTypeName);
        record.sourceIsPointer = isPointerType(event.sourceTypeName);
        record.targetIsPointer = isPointerType(event.targetTypeName);
        record.sourceIsEnum = isEnumType(event.sourceTypeName);
        record.targetIsEnum = isEnumType(event.targetTypeName);
        record.sourceIsSigned = isSignedType(event.sourceTypeName);
        record.targetIsSigned = isSignedType(event.targetTypeName);

        if (record.sourceIsBoolean || record.targetIsBoolean) {
            record.category = domain::conversion::ConversionCategory::BooleanConversion;
        } else if (record.sourceIsEnum || record.targetIsEnum) {
            record.category = domain::conversion::ConversionCategory::EnumConversion;
        } else if (isNarrowing(record.sourceType, record.targetType)) {
            record.category = domain::conversion::ConversionCategory::Narrowing;
        } else if (isWidening(record.sourceType, record.targetType)) {
            record.category = domain::conversion::ConversionCategory::Widening;
        } else if (record.sourceIsSigned != record.targetIsSigned) {
            record.category = domain::conversion::ConversionCategory::SignChange;
        } else if (record.sourceIsPointer || record.targetIsPointer) {
            record.category = domain::conversion::ConversionCategory::PointerConversion;
        } else {
            record.category = domain::conversion::ConversionCategory::Unknown;
        }

        return record;
    }
};

}  // namespace

ConversionClassifierPtr makeDefaultConversionClassifier() {
    return std::make_unique<DefaultConversionClassifier>();
}

}  // namespace discovery
