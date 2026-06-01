#include "fusion/FusionEngine.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace fusion {

// ============================================================================
// FindingID Implementation
// ============================================================================

FindingID FindingID::generate(const std::string& filePath, unsigned line, unsigned column) {
    std::ostringstream oss;

    // Extract filename from path
    size_t lastSlash = filePath.find_last_of("/\\");
    std::string filename = (lastSlash != std::string::npos)
        ? filePath.substr(lastSlash + 1)
        : filePath;

    // Generate ID: filename:line:column
    oss << filename << ":" << line << ":" << column;

    return FindingID{ oss.str() };
}

// ============================================================================
// FileLocation Implementation
// ============================================================================

std::string FileLocation::toString() const {
    std::ostringstream oss;
    oss << filePath << ":" << line << ":" << column;
    return oss.str();
}

// ============================================================================
// FusionRules Implementation
// ============================================================================

std::string FusionRules::applyContextDominance(
    const std::string& primaryContext,
    const std::vector<std::string>& secondaryContexts) {

    // Context dominance is already applied by Context Engine
    // This method validates and returns the dominant context
    return primaryContext;
}

std::uint32_t FusionRules::aggregateRisk(const RiskBreakdown& breakdown) {
    // Rule 2: Simple sum of all risk components
    return breakdown.total();
}

std::uint32_t FusionRules::enhanceForImpact(
    std::uint32_t baseScore,
    bool memoryAccess,
    bool controlFlow,
    bool apiBoundary) {

    // Rule 3: Impact Enhancement
    std::uint32_t enhanced = baseScore;

    if (memoryAccess) {
        enhanced += 20;  // MEMORY_ACCESS → +20
    }

    if (controlFlow) {
        enhanced += 10;  // CONTROL_FLOW → +10
    }

    if (apiBoundary) {
        enhanced += 15;  // API_BOUNDARY → +15
    }

    // Cap at 100
    return std::min(enhanced, 100u);
}

SeverityLevel FusionRules::mapSeverity(std::uint32_t finalScore) {
    // Rule 4: Severity Mapping
    if (finalScore <= 20) {
        return SeverityLevel::LOW;
    } else if (finalScore <= 40) {
        return SeverityLevel::MEDIUM;
    } else if (finalScore <= 70) {
        return SeverityLevel::HIGH;
    } else {
        return SeverityLevel::CRITICAL;
    }
}

// ============================================================================
// DefaultFusionEngine Implementation
// ============================================================================

class DefaultFusionEngine : public FusionEngine {
   public:
    Finding fuse(const FusionInput& input) override {
        Finding finding;

        // Generate Finding ID
        finding.id = FindingID::generate(
            input.conversion.location.filePath,
            input.conversion.location.line,
            input.conversion.location.column
        );

        // Set location
        finding.location.filePath = input.conversion.location.filePath;
        finding.location.line = input.conversion.location.line;
        finding.location.column = input.conversion.location.column;

        // Set conversion details
        finding.sourceType = input.conversion.sourceType;
        finding.targetType = input.conversion.targetType;
        finding.conversionCategory = conversionCategoryToString(input.conversion.category);

        // Set context information
        finding.primaryContext = context::contextTypeToString(
            input.contextClassification.primaryContext);

        for (const auto& ctx : input.contextClassification.secondaryContexts) {
            finding.secondaryContexts.push_back(context::contextTypeToString(ctx));
        }

        finding.contextConfidence = input.contextClassification.confidenceScore;

        // Set impact information
        finding.reachesMemoryAccess = input.propagationSummary.reachesMemoryAccess;
        finding.reachesControlFlow = input.propagationSummary.reachesControlFlow;
        finding.reachesAPIBoundary = input.propagationSummary.reachesAPIBoundary;

        // Build impact categories
        if (finding.reachesMemoryAccess) {
            finding.impactCategories.push_back("MEMORY_ACCESS");
        }
        if (finding.reachesControlFlow) {
            finding.impactCategories.push_back("CONTROL_FLOW");
        }
        if (finding.reachesAPIBoundary) {
            finding.impactCategories.push_back("API_BOUNDARY");
        }

        // Build impact paths
        for (const auto& path : input.propagationSummary.paths) {
            std::ostringstream pathStr;
            pathStr << "Path(" << path.pathNodes.size() << " nodes) → "
                    << impactCategoryToString(path.finalImpact.category);
            finding.impactPaths.push_back(pathStr.str());
        }

        // Set risk breakdown from Risk Engine
        finding.riskBreakdown.conversionRisk = input.riskAssessment.breakdown.conversionRisk;
        finding.riskBreakdown.contextRisk = input.riskAssessment.breakdown.contextRisk;
        finding.riskBreakdown.impactRisk = input.riskAssessment.breakdown.impactRisk;
        finding.riskBreakdown.propagationRisk = input.riskAssessment.breakdown.propagationRisk;

        // Compute final risk score with impact enhancement
        std::uint32_t baseScore = FusionRules::aggregateRisk(finding.riskBreakdown);
        finding.finalRiskScore = applyImpactEnhancement(baseScore, input.propagationSummary);

        // Compute severity
        finding.severity = computeSeverity(finding.finalRiskScore);

        // Generate explanation
        finding.explanation = generateExplanation(finding);

        // Generate fix suggestion
        finding.suggestedFix = generateFix(finding);

        // Set metadata
        finding.timestamp = getCurrentTimestamp();
        finding.analyzerVersion = "ICHA 1.0.0";

        return finding;
    }

    SeverityLevel computeSeverity(std::uint32_t finalRiskScore) const override {
        return FusionRules::mapSeverity(finalRiskScore);
    }

    std::string generateExplanation(const Finding& finding) const override {
        std::ostringstream oss;

        // Header
        oss << "Implicit conversion from '" << finding.sourceType
            << "' to '" << finding.targetType << "' detected at "
            << finding.location.toString() << ".\n\n";

        // Conversion category
        oss << "Conversion Category: " << finding.conversionCategory << "\n\n";

        // Context analysis
        oss << "Context Analysis:\n";
        oss << "  Primary Context: " << finding.primaryContext;
        if (!finding.secondaryContexts.empty()) {
            oss << "\n  Secondary Contexts: ";
            for (size_t i = 0; i < finding.secondaryContexts.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << finding.secondaryContexts[i];
            }
        }
        oss << "\n  Confidence: " << static_cast<int>(finding.contextConfidence * 100) << "%\n\n";

        // Impact analysis
        oss << "Impact Analysis:\n";
        if (finding.impactCategories.empty()) {
            oss << "  No downstream impact detected\n";
        } else {
            oss << "  Impact Categories: ";
            for (size_t i = 0; i < finding.impactCategories.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << finding.impactCategories[i];
            }
            oss << "\n";

            if (!finding.impactPaths.empty()) {
                oss << "  Propagation Paths: " << finding.impactPaths.size() << "\n";
                for (size_t i = 0; i < std::min(finding.impactPaths.size(), size_t(3)); ++i) {
                    oss << "    - " << finding.impactPaths[i] << "\n";
                }
            }
        }
        oss << "\n";

        // Risk breakdown
        oss << "Risk Assessment:\n";
        oss << "  Conversion Risk:  " << finding.riskBreakdown.conversionRisk << "\n";
        oss << "  Context Risk:     " << finding.riskBreakdown.contextRisk << "\n";
        oss << "  Impact Risk:      " << finding.riskBreakdown.impactRisk << "\n";
        oss << "  Propagation Risk: " << finding.riskBreakdown.propagationRisk << "\n";
        oss << "  ─────────────────────\n";
        oss << "  Base Score:       " << finding.riskBreakdown.total() << "\n";

        // Impact enhancement
        if (finding.finalRiskScore > finding.riskBreakdown.total()) {
            std::uint32_t enhancement = finding.finalRiskScore - finding.riskBreakdown.total();
            oss << "  Impact Enhancement: +" << enhancement << "\n";
        }

        oss << "  Final Risk Score: " << finding.finalRiskScore << "\n";
        oss << "  Severity:         " << severityToString(finding.severity) << "\n\n";

        // Reasoning
        oss << "Reasoning:\n";
        oss << buildReasoning(finding);

        return oss.str();
    }

    SuggestedFix generateFix(const Finding& finding) const override {
        SuggestedFix fix;

        // Generate description based on conversion type and context
        fix.description = generateFixDescription(finding);

        // Generate code snippet
        fix.codeSnippet = generateFixCodeSnippet(finding);

        // Generate steps
        fix.steps = generateFixSteps(finding);

        return fix;
    }

    std::uint32_t applyImpactEnhancement(
        std::uint32_t baseScore,
        const impact::PropagationSummary& propagation) const override {

        return FusionRules::enhanceForImpact(
            baseScore,
            propagation.reachesMemoryAccess,
            propagation.reachesControlFlow,
            propagation.reachesAPIBoundary
        );
    }

    std::string resolvePrimaryContext(
        const context::ContextClassification& classification) const override {

        return context::contextTypeToString(classification.primaryContext);
    }

   private:
    std::string conversionCategoryToString(domain::conversion::ConversionCategory category) const {
        using domain::conversion::ConversionCategory;
        switch (category) {
            case ConversionCategory::Narrowing:
                return "Narrowing";
            case ConversionCategory::Widening:
                return "Widening";
            case ConversionCategory::SignChange:
                return "SignChange";
            case ConversionCategory::EnumConversion:
                return "EnumConversion";
            case ConversionCategory::BooleanConversion:
                return "BooleanConversion";
            case ConversionCategory::PointerConversion:
                return "PointerConversion";
            case ConversionCategory::Unknown:
                return "Unknown";
        }
        return "Unknown";
    }

    std::string impactCategoryToString(impact::ImpactCategory category) const {
        switch (category) {
            case impact::ImpactCategory::MEMORY_ACCESS:
                return "MEMORY_ACCESS";
            case impact::ImpactCategory::CONTROL_FLOW:
                return "CONTROL_FLOW";
            case impact::ImpactCategory::API_BOUNDARY:
                return "API_BOUNDARY";
        }
        return "UNKNOWN";
    }

    std::string buildReasoning(const Finding& finding) const {
        std::ostringstream oss;

        // Context-based reasoning
        if (finding.primaryContext == "MEMORY_CONTEXT") {
            oss << "  - Conversion occurs in memory access context, posing direct memory safety risk\n";
        } else if (finding.primaryContext == "LOOP_CONTEXT") {
            oss << "  - Conversion affects loop control, potentially causing incorrect iteration bounds\n";
        } else if (finding.primaryContext == "BRANCH_CONTEXT") {
            oss << "  - Conversion influences conditional logic, may alter program control flow\n";
        } else if (finding.primaryContext == "API_CONTEXT") {
            oss << "  - Conversion crosses API boundary, affecting external interface contracts\n";
        }

        // Impact-based reasoning
        if (finding.reachesMemoryAccess) {
            oss << "  - Value propagates to memory access operations (array indexing, pointer dereference)\n";
        }
        if (finding.reachesControlFlow) {
            oss << "  - Value influences control flow decisions (loops, branches)\n";
        }
        if (finding.reachesAPIBoundary) {
            oss << "  - Value reaches external API boundaries\n";
        }

        // Severity reasoning
        if (finding.severity == SeverityLevel::CRITICAL) {
            oss << "  - CRITICAL severity: Immediate attention required, high risk of security vulnerability\n";
        } else if (finding.severity == SeverityLevel::HIGH) {
            oss << "  - HIGH severity: Significant risk, should be addressed promptly\n";
        } else if (finding.severity == SeverityLevel::MEDIUM) {
            oss << "  - MEDIUM severity: Moderate risk, review and fix recommended\n";
        } else {
            oss << "  - LOW severity: Minor risk, consider fixing during refactoring\n";
        }

        return oss.str();
    }

    std::string generateFixDescription(const Finding& finding) const {
        std::ostringstream oss;

        oss << "Use explicit static_cast<" << finding.targetType << ">(...) ";

        if (finding.conversionCategory == "SignChange") {
            oss << "with bounds checking to ensure value is within valid range";
        } else if (finding.conversionCategory == "Narrowing") {
            oss << "with range validation to prevent data loss";
        } else {
            oss << "to make the conversion explicit and document intent";
        }

        return oss.str();
    }

    std::string generateFixCodeSnippet(const Finding& finding) const {
        std::ostringstream oss;

        oss << "// Before:\n";
        oss << finding.targetType << " value = source;  // Implicit conversion\n\n";

        oss << "// After:\n";
        if (finding.conversionCategory == "SignChange" || finding.conversionCategory == "Narrowing") {
            oss << "// Add validation\n";
            oss << "if (source < 0 || source > MAX_VALUE) {\n";
            oss << "    // Handle error\n";
            oss << "}\n";
        }
        oss << finding.targetType << " value = static_cast<" << finding.targetType << ">(source);\n";

        return oss.str();
    }

    std::vector<std::string> generateFixSteps(const Finding& finding) const {
        std::vector<std::string> steps;

        steps.push_back("Replace implicit conversion with explicit static_cast");

        if (finding.conversionCategory == "SignChange") {
            steps.push_back("Add bounds checking to validate source value is non-negative");
        } else if (finding.conversionCategory == "Narrowing") {
            steps.push_back("Add range validation to ensure value fits in target type");
        }

        if (finding.reachesMemoryAccess) {
            steps.push_back("Validate converted value before using in memory operations");
        }

        steps.push_back("Add unit tests to verify conversion behavior");
        steps.push_back("Document the conversion rationale in code comments");

        return steps;
    }

    std::string getCurrentTimestamp() const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::ostringstream oss;
        oss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        return oss.str();
    }
};

// ============================================================================
// Factory Function
// ============================================================================

std::unique_ptr<FusionEngine> makeFusionEngine() {
    return std::make_unique<DefaultFusionEngine>();
}

}  // namespace fusion
