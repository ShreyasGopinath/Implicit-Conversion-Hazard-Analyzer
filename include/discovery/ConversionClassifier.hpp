#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "domain/conversion/ConversionRecord.hpp"
#include "parsing/ImplicitCastEvent.hpp"

namespace discovery {

class ConversionClassifier {
   public:
    virtual ~ConversionClassifier() = default;
    virtual domain::conversion::ConversionRecord classify(const parsing::ImplicitCastEvent& event) const = 0;
};

using ConversionClassifierPtr = std::unique_ptr<ConversionClassifier>;

ConversionClassifierPtr makeDefaultConversionClassifier();

}  // namespace discovery
