#pragma once

#include <string>
#include <vector>

namespace parsing {

enum class CastKind {
    Unknown,
    IntegralCast,
    FloatingCast,
    IntegralToBoolean,
    FloatingToIntegral,
    PointerCast,
    LValueToRValue,
    ArrayToPointer,
    FunctionToPointer
};

struct SourceLocationInfo {
    std::string filePath;
    unsigned line{ 0 };
    unsigned column{ 0 };
};

struct TypeTraits {
    std::string canonicalName;
    bool isPointer{ false };
    bool isBoolean{ false };
    bool isEnum{ false };
    bool isSigned{ false };
    bool isIntegral{ false };
    bool isFloating{ false };
    unsigned bitWidth{ 0 };
};

struct ASTMetadata {
    std::string parentNodeKind;
    std::vector<std::string> parentNodeChain;
    bool inLoopCondition{ false };
    bool inBranchCondition{ false };
    bool inArraySubscript{ false };
    bool inReturnStatement{ false };
    bool inApiBoundary{ false };
    bool inMemoryAllocation{ false };
    bool inArithmeticExpression{ false };
    bool inFunctionArgument{ false };
};

struct ImplicitCastEvent {
    std::string sourceTypeName;
    std::string targetTypeName;
    CastKind castKind{ CastKind::Unknown };
    SourceLocationInfo location;
    ASTMetadata metadata;
    TypeTraits sourceTraits;
    TypeTraits targetTraits;
    std::string astNodeDebug;
};

}  // namespace parsing
