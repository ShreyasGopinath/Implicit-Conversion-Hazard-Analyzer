#pragma once

#include <memory>

#include "context/ContextRecord.hpp"
#include "domain/conversion/ConversionRecord.hpp"
#include "risk/RiskFactors.hpp"
#include "risk/RiskScores.hpp"

namespace risk {

class RiskEngine {
   public:
    virtual ~RiskEngine() = default;

    virtual RiskAssessment assess(const domain::conversion::ConversionRecord& conversion,
                                  const context::ContextRecord& context,
                                  const ImpactProfile& impact,
                                  PropagationLevel propagation) const = 0;
};

std::unique_ptr<RiskEngine> makeRiskEngine();

}  // namespace risk
