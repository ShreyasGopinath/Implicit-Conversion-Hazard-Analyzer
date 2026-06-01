#pragma once

#include <string>
#include <vector>

#include "parsing/ImplicitCastEvent.hpp"

namespace context {

enum class ContextType {
    LOOP_CONTEXT,
    BRANCH_CONTEXT,
    MEMORY_CONTEXT,
    API_CONTEXT,
    RETURN_CONTEXT,
    ASSIGNMENT_CONTEXT,
    ARITHMETIC_CONTEXT,
    UNKNOWN_CONTEXT
};

struct ContextClassification {
    ContextType primaryContext{ ContextType::UNKNOWN_CONTEXT };
    std::vector<ContextType> secondaryContexts;
    float confidenceScore{ 0.0f };
    std::string semanticExplanation;
    std::vector<std::string> astContextPath;
};

struct EnhancedContextRecord {
    parsing::ImplicitCastEvent event;
    ContextClassification classification;

    // Legacy boolean flags for backward compatibility
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

inline std::string contextTypeToString(ContextType type) {
    switch (type) {
        case ContextType::LOOP_CONTEXT:
            return "LOOP_CONTEXT";
        case ContextType::BRANCH_CONTEXT:
            return "BRANCH_CONTEXT";
        case ContextType::MEMORY_CONTEXT:
            return "MEMORY_CONTEXT";
        case ContextType::API_CONTEXT:
            return "API_CONTEXT";
        case ContextType::RETURN_CONTEXT:
            return "RETURN_CONTEXT";
        case ContextType::ASSIGNMENT_CONTEXT:
            return "ASSIGNMENT_CONTEXT";
        case ContextType::ARITHMETIC_CONTEXT:
            return "ARITHMETIC_CONTEXT";
        case ContextType::UNKNOWN_CONTEXT:
            return "UNKNOWN_CONTEXT";
    }
    return "UNKNOWN_CONTEXT";
}

}  // namespace context
