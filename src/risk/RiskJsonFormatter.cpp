#include "risk/RiskJsonFormatter.hpp"

#include <sstream>
#include <string>

#include "risk/RiskScores.hpp"

namespace risk {
namespace {
std::string severityToString(Severity severity) {
    switch (severity) {
        case Severity::Low:
            return "LOW";
        case Severity::Medium:
            return "MEDIUM";
        case Severity::High:
            return "HIGH";
        case Severity::Critical:
            return "CRITICAL";
    }
    return "LOW";
}

std::string propagationToString(PropagationLevel level) {
    switch (level) {
        case PropagationLevel::ImmediateUse:
            return "IMMEDIATE_USE";
        case PropagationLevel::SameExpressionChain:
            return "SAME_EXPRESSION";
        case PropagationLevel::CrossFunction:
            return "CROSS_FUNCTION";
        case PropagationLevel::ApiBoundary:
            return "API_BOUNDARY";
        case PropagationLevel::ModuleBoundary:
            return "MODULE_BOUNDARY";
        case PropagationLevel::None:
        default:
            return "NONE";
    }
}

std::string escape(const std::string& value) {
    std::string result;
    result.reserve(value.size() + 8);
    for (char ch : value) {
        switch (ch) {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += ch;
        }
    }
    return result;
}

class DefaultRiskJsonFormatter : public RiskJsonFormatter {
   public:
    std::string format(const RiskAssessment& assessment) const override {
        std::ostringstream oss;
        oss << '{';

        oss << "\"conversion\":{\"source\":\"" << escape(assessment.conversion.sourceType)
            << "\",\"target\":\"" << escape(assessment.conversion.targetType) << "\"},";

        oss << "\"context\":{"
            << "\"loop_condition\":" << std::boolalpha << assessment.context.inLoopCondition << ','
            << "\"branch_condition\":" << assessment.context.inBranchCondition << ','
            << "\"array_index\":" << assessment.context.inArrayIndex << "},";

        oss << "\"impact\":{"
            << "\"memory_access\":" << assessment.impact.affectsMemoryAccess << ','
            << "\"control_flow\":" << assessment.impact.affectsControlFlow << "},";

        oss << "\"propagation\":\"" << propagationToString(assessment.propagation) << "\",";

        oss << "\"risk_breakdown\":{\"conversion\":" << assessment.breakdown.conversionRisk
            << ",\"context\":" << assessment.breakdown.contextRisk
            << ",\"impact\":" << assessment.breakdown.impactRisk
            << ",\"propagation\":" << assessment.breakdown.propagationRisk
            << ",\"total\":" << assessment.breakdown.total() << "},";

        oss << "\"severity\":\"" << severityToString(assessment.severity) << "\",";
        oss << "\"explanation\":\"" << escape(assessment.explanation) << "\",";
        oss << "\"suggested_fix\":\"" << escape(assessment.suggestedFix) << "\"";
        oss << '}';
        return oss.str();
    }
};
}  // namespace

std::unique_ptr<RiskJsonFormatter> makeRiskJsonFormatter() {
    return std::make_unique<DefaultRiskJsonFormatter>();
}

}  // namespace risk
