#pragma once

#include <functional>
#include <vector>

#include "domain/conversion/ConversionRecord.hpp"
#include "parsing/ImplicitCastEvent.hpp"

namespace discovery {

class ConversionDiscoveryEngine {
   public:
    using RecordCallback = std::function<void(const domain::conversion::ConversionRecord&)>;

    virtual ~ConversionDiscoveryEngine() = default;
    virtual void process(const parsing::ImplicitCastEvent& event, RecordCallback callback) const = 0;
};

std::unique_ptr<ConversionDiscoveryEngine> makeConversionDiscoveryEngine();

}  // namespace discovery
