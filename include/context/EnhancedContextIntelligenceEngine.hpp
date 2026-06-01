#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "context/ContextClassification.hpp"
#include "context/ContextRecord.hpp"
#include "parsing/ImplicitCastEvent.hpp"

namespace context {

class EnhancedContextIntelligenceEngine {
   public:
    using Callback = std::function<void(const EnhancedContextRecord&)>;

    virtual ~EnhancedContextIntelligenceEngine() = default;

    // Main analysis entry point
    virtual void analyze(const parsing::ImplicitCastEvent& event, Callback callback) const = 0;

    // Advanced classification methods
    virtual ContextClassification classifyContext(const parsing::ImplicitCastEvent& event) const = 0;

    // AST context path extraction
    virtual std::vector<std::string> getASTContextPath(const parsing::ImplicitCastEvent& event) const = 0;

    // Explanation generation
    virtual std::string generateExplanation(const ContextClassification& classification,
                                           const parsing::ImplicitCastEvent& event) const = 0;

    // Confidence scoring
    virtual float calculateConfidence(const ContextClassification& classification) const = 0;
};

// Factory function for enhanced engine
std::unique_ptr<EnhancedContextIntelligenceEngine> makeEnhancedContextIntelligenceEngine();

// Priority-based context resolver
class ContextPriorityResolver {
   public:
    // Apply priority rules to determine primary context
    static ContextType resolvePrimaryContext(const std::vector<ContextType>& contexts);

    // Get priority value for a context type (higher = more important)
    static int getPriority(ContextType type);

    // Check if context A should override context B
    static bool shouldOverride(ContextType a, ContextType b);
};

}  // namespace context
