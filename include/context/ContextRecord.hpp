#pragma once

#include <string>

#include "parsing/ImplicitCastEvent.hpp"

namespace context {

enum class ContextTag {
    LoopCondition,
    BranchCondition,
    ArrayIndex,
    MemoryAllocation,
    ApiBoundary,
    ArithmeticExpression,
    ReturnStatement
};

struct ContextRecord {
    parsing::ImplicitCastEvent event;
    bool inLoopCondition{ false };
    bool inBranchCondition{ false };
    bool inArrayIndex{ false };
    bool inMemoryAllocation{ false };
    bool inApiBoundary{ false };
    bool inArithmeticExpression{ false };
    bool inReturnStatement{ false };
    bool inFunctionArgument{ false };
    bool isAssignmentOnly{ false };
};

}  // namespace context
