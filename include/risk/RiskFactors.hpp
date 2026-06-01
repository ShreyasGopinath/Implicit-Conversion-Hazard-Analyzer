#pragma once

#include <cstdint>

namespace risk {

enum class PropagationLevel {
    None,
    ImmediateUse,
    SameExpressionChain,
    CrossFunction,
    ModuleBoundary,
    ApiBoundary
};

struct ImpactProfile {
    bool affectsMemoryAccess{ false };
    bool affectsControlFlow{ false };
    bool affectsArithmetic{ false };
};

}  // namespace risk
