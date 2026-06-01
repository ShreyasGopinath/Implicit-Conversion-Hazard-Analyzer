#pragma once

#include <cstdint>
#include <string>

#include "context/ContextRecord.hpp"
#include "domain/conversion/ConversionRecord.hpp"
#include "risk/RiskFactors.hpp"

namespace risk {

struct RiskBreakdown {
    std::uint32_t conversionRisk{ 0 };
    std::uint32_t contextRisk{ 0 };
    std::uint32_t impactRisk{ 0 };
    std::uint32_t propagationRisk{ 0 };

    std::uint32_t total() const { return conversionRisk + contextRisk + impactRisk + propagationRisk; }
};

enum class Severity {
    Low,
    Medium,
    High,
    Critical
};

struct RiskAssessment {
    domain::conversion::ConversionRecord conversion;
    context::ContextRecord context;
    ImpactProfile impact;
    PropagationLevel propagation{ PropagationLevel::None };
    RiskBreakdown breakdown;
    Severity severity{ Severity::Low };
    std::string explanation;
    std::string suggestedFix;
    std::string json;
};

}  // namespace risk
