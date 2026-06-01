#pragma once

#include <functional>
#include <memory>

#include "context/ContextRecord.hpp"
#include "parsing/ImplicitCastEvent.hpp"

namespace context {

class ContextIntelligenceEngine {
   public:
    using Callback = std::function<void(const ContextRecord&)>;

    virtual ~ContextIntelligenceEngine() = default;
    virtual void analyze(const parsing::ImplicitCastEvent& event, Callback callback) const = 0;
};

std::unique_ptr<ContextIntelligenceEngine> makeContextIntelligenceEngine();

}  // namespace context
