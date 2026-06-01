#pragma once

#include <string>
#include <vector>

namespace domain::conversion {

enum class ConversionCategory {
    Narrowing,
    Widening,
    SignChange,
    EnumConversion,
    BooleanConversion,
    PointerConversion,
    Unknown
};

struct ConversionRecord {
    struct SourceLocation {
        std::string filePath;
        unsigned line{ 0 };
        unsigned column{ 0 };
    } location;

    struct ContextSnapshot {
        bool inLoopCondition{ false };
        bool inBranchCondition{ false };
        bool inArrayIndex{ false };
        bool inMemoryAllocation{ false };
        bool inApiBoundary{ false };
        bool inReturnStatement{ false };
        bool inFunctionArgument{ false };
        bool inArithmeticExpression{ false };
    } context;

    std::string sourceType;
    std::string targetType;
    ConversionCategory category{ ConversionCategory::Unknown };
    bool sourceIsSigned{ false };
    bool targetIsSigned{ false };
    bool sourceIsEnum{ false };
    bool targetIsEnum{ false };
    bool sourceIsPointer{ false };
    bool targetIsPointer{ false };
    bool sourceIsBoolean{ false };
    bool targetIsBoolean{ false };
    std::string astParentNode;
};

}  // namespace domain::conversion
