#pragma once

#include <memory>
#include <string>

#include "risk/RiskScores.hpp"

namespace risk {

class RiskJsonFormatter {
   public:
    virtual ~RiskJsonFormatter() = default;
    virtual std::string format(const RiskAssessment& assessment) const = 0;
};

std::unique_ptr<RiskJsonFormatter> makeRiskJsonFormatter();

}  // namespace risk
