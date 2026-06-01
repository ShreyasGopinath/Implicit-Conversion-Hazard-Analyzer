#include "risk/RiskEngine.hpp"

#include <sstream>

#include "risk/RiskJsonFormatter.hpp"

namespace risk {
namespace {

bool containsType(const std::string& typeName, const std::string& keyword) {
    return typeName.find(keyword) != std::string::npos;
}

bool isIntType(const std::string& typeName) {
    return typeName == "int" || typeName == "signed int";
}

bool isShortType(const std::string& typeName) {
    return containsType(typeName, "short");
}

bool isLongType(const std::string& typeName) {
    return containsType(typeName, "long") && !containsType(typeName, "long long");
}

bool isPointerType(const std::string& typeName) {
    return containsType(typeName, "*");
}

bool isIntegralType(const std::string& typeName) {
    return isIntType(typeName) || isShortType(typeName) || isLongType(typeName) ||
           containsType(typeName, "char") || containsType(typeName, "long long");
}

std::uint32_t computeConversionRisk(const domain::conversion::ConversionRecord& conversion) {
    // double → int = 25
    if (conversion.sourceType == "double" && isIntType(conversion.targetType)) {
        return 25;
    }

    // float → int = 20
    if (conversion.sourceType == "float" && isIntType(conversion.targetType)) {
        return 20;
    }

    // long → int = 15
    if (isLongType(conversion.sourceType) && isIntType(conversion.targetType)) {
        return 15;
    }

    // int → short = 15
    if (isIntType(conversion.sourceType) && isShortType(conversion.targetType)) {
        return 15;
    }

    // signed → unsigned = 30
    if (conversion.sourceIsSigned && !conversion.targetIsSigned &&
        !conversion.sourceIsPointer && !conversion.targetIsPointer) {
        return 30;
    }

    // unsigned → signed = 25
    if (!conversion.sourceIsSigned && conversion.targetIsSigned &&
        !conversion.sourceIsPointer && !conversion.targetIsPointer) {
        return 25;
    }

    // enum → int = 10
    if (conversion.sourceIsEnum && isIntegralType(conversion.targetType)) {
        return 10;
    }

    // int → enum = 15
    if (isIntegralType(conversion.sourceType) && conversion.targetIsEnum) {
        return 15;
    }

    // pointer → integer = 35
    if (isPointerType(conversion.sourceType) && isIntegralType(conversion.targetType)) {
        return 35;
    }

    return 0;
}

std::uint32_t computeContextRisk(const context::ContextRecord& context) {
    // Contexts are mutually exclusive - return the highest priority match

    // array indexing context = 30 (highest priority - most dangerous)
    if (context.inArrayIndex) {
        return 30;
    }

    // loop condition (for/while boundary) = 25
    if (context.inLoopCondition) {
        return 25;
    }

    // return statement = 15
    if (context.inReturnStatement) {
        return 15;
    }

    // branch condition (if/switch) = 15
    if (context.inBranchCondition) {
        return 15;
    }

    // function argument passing = 10
    if (context.inFunctionArgument) {
        return 10;
    }

    // arithmetic expression = 10
    if (context.inArithmeticExpression) {
        return 10;
    }

    // assignment only = 5 (lowest risk)
    if (context.isAssignmentOnly) {
        return 5;
    }

    return 0;
}

std::uint32_t computeImpactRisk(const ImpactProfile& impact) {
    // Impacts can overlap - use highest priority match

    // affects memory access (array, pointer dereference) = 30
    if (impact.affectsMemoryAccess) {
        return 30;
    }

    // affects control flow decision = 20
    if (impact.affectsControlFlow) {
        return 20;
    }

    // affects arithmetic computation = 10
    if (impact.affectsArithmetic) {
        return 10;
    }

    // no downstream effect = 0
    return 0;
}

std::uint32_t computePropagationRisk(PropagationLevel propagation) {
    switch (propagation) {
        // immediate use after conversion = 5
        case PropagationLevel::ImmediateUse:
            return 5;

        // used in same expression chain = 10
        case PropagationLevel::SameExpressionChain:
            return 10;

        // used in different function = 20
        case PropagationLevel::CrossFunction:
            return 20;

        // crosses module boundary = 25
        case PropagationLevel::ModuleBoundary:
            return 25;

        // reaches API boundary (system/library call) = 30
        case PropagationLevel::ApiBoundary:
            return 30;

        case PropagationLevel::None:
        default:
            return 0;
    }
}

Severity mapSeverity(std::uint32_t total) {
    // 0–20 = LOW
    if (total <= 20) {
        return Severity::Low;
    }
    // 21–40 = MEDIUM
    if (total <= 40) {
        return Severity::Medium;
    }
    // 41–70 = HIGH
    if (total <= 70) {
        return Severity::High;
    }
    // 71–100 = CRITICAL
    return Severity::Critical;
}

std::string buildExplanation(const domain::conversion::ConversionRecord& conversion,
                             const context::ContextRecord& context,
                             const RiskBreakdown& breakdown,
                             const ImpactProfile& impact,
                             PropagationLevel propagation) {
    std::ostringstream oss;

    // Conversion description
    oss << "Implicit conversion from '" << conversion.sourceType
        << "' to '" << conversion.targetType << "' detected. ";

    // Risk breakdown
    oss << "Risk breakdown: ";
    oss << "Conversion=" << breakdown.conversionRisk;
    oss << ", Context=" << breakdown.contextRisk;
    oss << ", Impact=" << breakdown.impactRisk;
    oss << ", Propagation=" << breakdown.propagationRisk;
    oss << " (Total=" << breakdown.total() << "). ";

    // Context explanation
    if (context.inArrayIndex) {
        oss << "Used in array indexing (high memory safety risk). ";
    }
    if (context.inLoopCondition) {
        oss << "Occurs in loop condition (affects iteration bounds). ";
    }
    if (context.inBranchCondition) {
        oss << "Used in branch condition (affects control flow). ";
    }
    if (context.inReturnStatement) {
        oss << "Used in return statement (affects function contract). ";
    }
    if (context.inFunctionArgument) {
        oss << "Passed as function argument (crosses function boundary). ";
    }
    if (context.inArithmeticExpression) {
        oss << "Used in arithmetic expression (may affect computation accuracy). ";
    }
    if (context.inMemoryAllocation) {
        oss << "Used in memory allocation (critical for memory safety). ";
    }
    if (context.inApiBoundary) {
        oss << "Crosses API boundary (external interface risk). ";
    }

    // Impact explanation
    if (impact.affectsMemoryAccess) {
        oss << "Affects memory access operations. ";
    }
    if (impact.affectsControlFlow) {
        oss << "Affects control flow decisions. ";
    }
    if (impact.affectsArithmetic) {
        oss << "Affects arithmetic computations. ";
    }

    // Propagation explanation
    switch (propagation) {
        case PropagationLevel::ApiBoundary:
            oss << "Value propagates to API boundary.";
            break;
        case PropagationLevel::ModuleBoundary:
            oss << "Value crosses module boundary.";
            break;
        case PropagationLevel::CrossFunction:
            oss << "Value propagates across functions.";
            break;
        case PropagationLevel::SameExpressionChain:
            oss << "Value used in same expression chain.";
            break;
        case PropagationLevel::ImmediateUse:
            oss << "Value used immediately after conversion.";
            break;
        case PropagationLevel::None:
            break;
    }

    return oss.str();
}

std::string buildSuggestedFix(const domain::conversion::ConversionRecord& conversion,
                              const context::ContextRecord& context) {
    std::ostringstream oss;

    // Floating point to integer conversions
    if (conversion.sourceType == "double" && isIntType(conversion.targetType)) {
        oss << "Use explicit static_cast<int>(...) with range validation. ";
        oss << "Consider using std::lround() or std::floor() for controlled rounding. ";
        if (context.inArrayIndex) {
            oss << "Validate result is non-negative before array indexing.";
        }
        return oss.str();
    }

    if (conversion.sourceType == "float" && isIntType(conversion.targetType)) {
        oss << "Use explicit static_cast<int>(...) with range validation. ";
        oss << "Consider using std::lroundf() for controlled rounding. ";
        if (context.inArrayIndex) {
            oss << "Validate result is non-negative before array indexing.";
        }
        return oss.str();
    }

    // Narrowing conversions
    if (isLongType(conversion.sourceType) && isIntType(conversion.targetType)) {
        oss << "Use explicit static_cast<int>(...) and validate value fits in target range. ";
        oss << "Consider using gsl::narrow_cast or similar checked conversion.";
        return oss.str();
    }

    if (isIntType(conversion.sourceType) && isShortType(conversion.targetType)) {
        oss << "Use explicit static_cast<short>(...) with range checking. ";
        oss << "Ensure value is within [-32768, 32767] before conversion.";
        return oss.str();
    }

    // Sign conversions
    if (conversion.sourceIsSigned && !conversion.targetIsSigned) {
        oss << "Use explicit static_cast with bounds checking. ";
        oss << "Validate source value is non-negative before converting to unsigned type. ";
        if (context.inArrayIndex || context.inMemoryAllocation) {
            oss << "Critical: negative values will wrap to large positive values.";
        }
        return oss.str();
    }

    if (!conversion.sourceIsSigned && conversion.targetIsSigned) {
        oss << "Use explicit static_cast and ensure unsigned value fits in signed range. ";
        oss << "Check that value does not exceed maximum signed value.";
        return oss.str();
    }

    // Enum conversions
    if (conversion.sourceIsEnum && isIntegralType(conversion.targetType)) {
        oss << "Use explicit static_cast<" << conversion.targetType << ">(...). ";
        oss << "Consider if enum should be used directly instead of converting.";
        return oss.str();
    }

    if (isIntegralType(conversion.sourceType) && conversion.targetIsEnum) {
        oss << "Use explicit static_cast<" << conversion.targetType << ">(...). ";
        oss << "Validate integer value corresponds to valid enum value. ";
        oss << "Consider using enum class for type safety.";
        return oss.str();
    }

    // Pointer to integer
    if (isPointerType(conversion.sourceType) && isIntegralType(conversion.targetType)) {
        oss << "Use reinterpret_cast<" << conversion.targetType << ">(...) to make intent explicit. ";
        oss << "Consider using std::uintptr_t for portable pointer-to-integer conversions. ";
        oss << "Warning: pointer-to-integer conversions are platform-dependent.";
        return oss.str();
    }

    // Generic fallback
    oss << "Use explicit static_cast<" << conversion.targetType << ">(...) to document intent. ";
    oss << "Validate value ranges before conversion to prevent data loss.";
    return oss.str();
}

class DefaultRiskEngine : public RiskEngine {
   public:
    RiskAssessment assess(const domain::conversion::ConversionRecord& conversion,
                          const context::ContextRecord& context,
                          const ImpactProfile& impact,
                          PropagationLevel propagation) const override {
        RiskAssessment assessment{};
        assessment.conversion = conversion;
        assessment.context = context;
        assessment.impact = impact;
        assessment.propagation = propagation;

        // Compute risk scores according to specification
        assessment.breakdown.conversionRisk = computeConversionRisk(conversion);
        assessment.breakdown.contextRisk = computeContextRisk(context);
        assessment.breakdown.impactRisk = computeImpactRisk(impact);
        assessment.breakdown.propagationRisk = computePropagationRisk(propagation);

        // Map total score to severity level
        assessment.severity = mapSeverity(assessment.breakdown.total());

        // Generate explanation and suggested fix
        assessment.explanation = buildExplanation(conversion, context, assessment.breakdown, impact, propagation);
        assessment.suggestedFix = buildSuggestedFix(conversion, context);

        // Format as JSON
        assessment.json = makeRiskJsonFormatter()->format(assessment);

        return assessment;
    }
};
}  // namespace

std::unique_ptr<RiskEngine> makeRiskEngine() {
    return std::make_unique<DefaultRiskEngine>();
}

}  // namespace risk
