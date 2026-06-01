#pragma once

#include <memory>
#include <vector>

#include "fusion/Finding.hpp"
#include "context/ContextClassification.hpp"
#include "domain/conversion/ConversionRecord.hpp"
#include "impact/ImpactPath.hpp"
#include "risk/RiskScores.hpp"

namespace fusion {

// Input bundle for fusion
struct FusionInput {
    domain::conversion::ConversionRecord conversion;
    context::ContextClassification contextClassification;
    impact::PropagationSummary propagationSummary;
    risk::RiskAssessment riskAssessment;
};

class FusionEngine {
   public:
    virtual ~FusionEngine() = default;

    // Main fusion method
    virtual Finding fuse(const FusionInput& input) = 0;

    // Individual fusion components
    virtual SeverityLevel computeSeverity(std::uint32_t finalRiskScore) const = 0;

    virtual std::string generateExplanation(const Finding& finding) const = 0;

    virtual SuggestedFix generateFix(const Finding& finding) const = 0;

    // Impact enhancement
    virtual std::uint32_t applyImpactEnhancement(
        std::uint32_t baseScore,
        const impact::PropagationSummary& propagation) const = 0;

    // Context resolution
    virtual std::string resolvePrimaryContext(
        const context::ContextClassification& classification) const = 0;
};

// Factory function
std::unique_ptr<FusionEngine> makeFusionEngine();

// Fusion rules implementation
class FusionRules {
   public:
    // Rule 1: Context Dominance
    static std::string applyContextDominance(
        const std::string& primaryContext,
        const std::vector<std::string>& secondaryContexts);

    // Rule 2: Risk Aggregation
    static std::uint32_t aggregateRisk(const RiskBreakdown& breakdown);

    // Rule 3: Impact Enhancement
    static std::uint32_t enhanceForImpact(
        std::uint32_t baseScore,
        bool memoryAccess,
        bool controlFlow,
        bool apiBoundary);

    // Rule 4: Severity Mapping
    static SeverityLevel mapSeverity(std::uint32_t finalScore);
};

}  // namespace fusion
