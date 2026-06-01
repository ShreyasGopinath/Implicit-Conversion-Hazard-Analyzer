#include "context/EnhancedContextIntelligenceEngine.hpp"

#include <algorithm>
#include <sstream>
#include <unordered_set>

namespace context {

// ============================================================================
// ContextPriorityResolver Implementation
// ============================================================================

int ContextPriorityResolver::getPriority(ContextType type) {
    // Higher number = higher priority
    switch (type) {
        case ContextType::MEMORY_CONTEXT:
            return 100;  // Highest priority
        case ContextType::LOOP_CONTEXT:
            return 80;
        case ContextType::API_CONTEXT:
            return 70;
        case ContextType::RETURN_CONTEXT:
            return 60;
        case ContextType::BRANCH_CONTEXT:
            return 50;
        case ContextType::ARITHMETIC_CONTEXT:
            return 30;
        case ContextType::ASSIGNMENT_CONTEXT:
            return 20;  // Lowest priority
        case ContextType::UNKNOWN_CONTEXT:
            return 0;
    }
    return 0;
}

bool ContextPriorityResolver::shouldOverride(ContextType a, ContextType b) {
    return getPriority(a) > getPriority(b);
}

ContextType ContextPriorityResolver::resolvePrimaryContext(const std::vector<ContextType>& contexts) {
    if (contexts.empty()) {
        return ContextType::UNKNOWN_CONTEXT;
    }

    // Find context with highest priority
    ContextType primary = contexts[0];
    for (const auto& ctx : contexts) {
        if (shouldOverride(ctx, primary)) {
            primary = ctx;
        }
    }

    return primary;
}

// ============================================================================
// Enhanced Context Intelligence Engine Implementation
// ============================================================================

class DefaultEnhancedContextIntelligenceEngine : public EnhancedContextIntelligenceEngine {
   public:
    void analyze(const parsing::ImplicitCastEvent& event, Callback callback) const override {
        if (!callback) {
            return;
        }

        EnhancedContextRecord record{};
        record.event = event;
        record.classification = classifyContext(event);

        // Populate legacy boolean flags for backward compatibility
        populateLegacyFlags(record);

        callback(record);
    }

    ContextClassification classifyContext(const parsing::ImplicitCastEvent& event) const override {
        ContextClassification classification;

        // Collect all applicable contexts
        std::vector<ContextType> detectedContexts;

        // Check each context type based on AST metadata
        if (isMemoryContext(event)) {
            detectedContexts.push_back(ContextType::MEMORY_CONTEXT);
        }

        if (isLoopContext(event)) {
            detectedContexts.push_back(ContextType::LOOP_CONTEXT);
        }

        if (isBranchContext(event)) {
            detectedContexts.push_back(ContextType::BRANCH_CONTEXT);
        }

        if (isAPIContext(event)) {
            detectedContexts.push_back(ContextType::API_CONTEXT);
        }

        if (isReturnContext(event)) {
            detectedContexts.push_back(ContextType::RETURN_CONTEXT);
        }

        if (isArithmeticContext(event)) {
            detectedContexts.push_back(ContextType::ARITHMETIC_CONTEXT);
        }

        if (isAssignmentContext(event)) {
            detectedContexts.push_back(ContextType::ASSIGNMENT_CONTEXT);
        }

        // Apply priority rules to determine primary context
        classification.primaryContext = ContextPriorityResolver::resolvePrimaryContext(detectedContexts);

        // Store secondary contexts (all except primary)
        for (const auto& ctx : detectedContexts) {
            if (ctx != classification.primaryContext) {
                classification.secondaryContexts.push_back(ctx);
            }
        }

        // Calculate confidence score
        classification.confidenceScore = calculateConfidence(classification);

        // Generate AST context path
        classification.astContextPath = getASTContextPath(event);

        // Generate semantic explanation
        classification.semanticExplanation = generateExplanation(classification, event);

        return classification;
    }

    std::vector<std::string> getASTContextPath(const parsing::ImplicitCastEvent& event) const override {
        std::vector<std::string> path;

        // Build path from AST metadata
        path.push_back("ImplicitCastExpr");

        if (!event.metadata.parentNodeKind.empty()) {
            path.push_back(event.metadata.parentNodeKind);
        }

        // Add context-specific nodes
        if (event.metadata.inArraySubscript) {
            path.push_back("ArraySubscriptExpr");
        }

        if (event.metadata.inLoopCondition) {
            path.push_back("LoopStmt");
        }

        if (event.metadata.inBranchCondition) {
            path.push_back("IfStmt/SwitchStmt");
        }

        if (event.metadata.inReturnStatement) {
            path.push_back("ReturnStmt");
        }

        if (event.metadata.inApiBoundary) {
            path.push_back("CallExpr");
        }

        return path;
    }

    std::string generateExplanation(const ContextClassification& classification,
                                    const parsing::ImplicitCastEvent& event) const override {
        std::ostringstream oss;

        // Primary context explanation
        oss << "Primary Context: " << contextTypeToString(classification.primaryContext);

        switch (classification.primaryContext) {
            case ContextType::MEMORY_CONTEXT:
                oss << " - Conversion affects memory access operation";
                if (event.metadata.inArraySubscript) {
                    oss << " (array indexing)";
                } else if (event.metadata.inMemoryAllocation) {
                    oss << " (memory allocation)";
                } else {
                    oss << " (pointer dereference)";
                }
                break;

            case ContextType::LOOP_CONTEXT:
                oss << " - Conversion occurs in loop control structure";
                oss << ", affecting iteration bounds or conditions";
                break;

            case ContextType::BRANCH_CONTEXT:
                oss << " - Conversion affects conditional branching logic";
                break;

            case ContextType::API_CONTEXT:
                oss << " - Conversion crosses API/function boundary";
                if (event.metadata.inFunctionArgument) {
                    oss << " (function argument)";
                }
                break;

            case ContextType::RETURN_CONTEXT:
                oss << " - Conversion occurs in return statement";
                oss << ", affecting function contract";
                break;

            case ContextType::ARITHMETIC_CONTEXT:
                oss << " - Conversion used in arithmetic expression";
                break;

            case ContextType::ASSIGNMENT_CONTEXT:
                oss << " - Simple assignment context";
                break;

            case ContextType::UNKNOWN_CONTEXT:
                oss << " - Context could not be determined";
                break;
        }

        // Add secondary contexts if present
        if (!classification.secondaryContexts.empty()) {
            oss << ". Secondary contexts: ";
            for (size_t i = 0; i < classification.secondaryContexts.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << contextTypeToString(classification.secondaryContexts[i]);
            }
        }

        // Add confidence information
        oss << ". Confidence: " << static_cast<int>(classification.confidenceScore * 100) << "%";

        return oss.str();
    }

    float calculateConfidence(const ContextClassification& classification) const override {
        // Base confidence on number of detected contexts and clarity
        float confidence = 0.5f;  // Base confidence

        // High confidence for clear, single-context scenarios
        if (classification.secondaryContexts.empty()) {
            confidence = 0.95f;
        }
        // Medium confidence for primary + 1 secondary
        else if (classification.secondaryContexts.size() == 1) {
            confidence = 0.80f;
        }
        // Lower confidence for multiple overlapping contexts
        else if (classification.secondaryContexts.size() == 2) {
            confidence = 0.70f;
        }
        // Even lower for complex scenarios
        else {
            confidence = 0.60f;
        }

        // Boost confidence for high-priority contexts
        if (classification.primaryContext == ContextType::MEMORY_CONTEXT) {
            confidence = std::min(1.0f, confidence + 0.05f);
        }

        // Reduce confidence for unknown context
        if (classification.primaryContext == ContextType::UNKNOWN_CONTEXT) {
            confidence = 0.3f;
        }

        return confidence;
    }

   private:
    // Context detection methods based on AST metadata

    bool isMemoryContext(const parsing::ImplicitCastEvent& event) const {
        return event.metadata.inArraySubscript ||
               event.metadata.inMemoryAllocation ||
               isPointerDereference(event);
    }

    bool isLoopContext(const parsing::ImplicitCastEvent& event) const {
        return event.metadata.inLoopCondition;
    }

    bool isBranchContext(const parsing::ImplicitCastEvent& event) const {
        return event.metadata.inBranchCondition;
    }

    bool isAPIContext(const parsing::ImplicitCastEvent& event) const {
        return event.metadata.inApiBoundary ||
               event.metadata.inFunctionArgument;
    }

    bool isReturnContext(const parsing::ImplicitCastEvent& event) const {
        return event.metadata.inReturnStatement;
    }

    bool isArithmeticContext(const parsing::ImplicitCastEvent& event) const {
        return event.metadata.inArithmeticExpression;
    }

    bool isAssignmentContext(const parsing::ImplicitCastEvent& event) const {
        // Assignment context is the default if no other specific context is detected
        return !event.metadata.inLoopCondition &&
               !event.metadata.inBranchCondition &&
               !event.metadata.inArraySubscript &&
               !event.metadata.inReturnStatement &&
               !event.metadata.inApiBoundary &&
               !event.metadata.inMemoryAllocation &&
               !event.metadata.inFunctionArgument;
    }

    bool isPointerDereference(const parsing::ImplicitCastEvent& event) const {
        // Check if any parent node suggests pointer dereference
        for (const auto& parent : event.metadata.parentNodeChain) {
            if (parent.find("UnaryOperator") != std::string::npos ||
                parent.find("MemberExpr") != std::string::npos) {
                return true;
            }
        }
        return false;
    }

    void populateLegacyFlags(EnhancedContextRecord& record) const {
        const auto& event = record.event;
        const auto& classification = record.classification;

        // Set legacy flags based on detected contexts
        record.inLoopCondition = event.metadata.inLoopCondition;
        record.inBranchCondition = event.metadata.inBranchCondition;
        record.inArrayIndex = event.metadata.inArraySubscript;
        record.inMemoryAllocation = event.metadata.inMemoryAllocation;
        record.inApiBoundary = event.metadata.inApiBoundary;
        record.inArithmeticExpression = event.metadata.inArithmeticExpression;
        record.inReturnStatement = event.metadata.inReturnStatement;
        record.inFunctionArgument = event.metadata.inFunctionArgument;

        // isAssignmentOnly is true only if it's the primary context
        record.isAssignmentOnly = (classification.primaryContext == ContextType::ASSIGNMENT_CONTEXT);
    }
};

// ============================================================================
// Factory Function
// ============================================================================

std::unique_ptr<EnhancedContextIntelligenceEngine> makeEnhancedContextIntelligenceEngine() {
    return std::make_unique<DefaultEnhancedContextIntelligenceEngine>();
}

}  // namespace context
