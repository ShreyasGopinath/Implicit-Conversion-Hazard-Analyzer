#include "reporting/ReportingEngine.hpp"

#include <algorithm>
#include <sstream>
#include <map>

namespace reporting {

// ============================================================================
// Constructor
// ============================================================================

ReportingEngine::ReportingEngine(const ReportConfig& config)
    : config_(config) {}

// ============================================================================
// Sorting and Grouping
// ============================================================================

std::vector<fusion::Finding> ReportingEngine::sortFindings(
    const std::vector<fusion::Finding>& findings) {

    auto sorted = findings;

    // Sort by: file path, then line, then severity (descending)
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        if (a.location.filePath != b.location.filePath) {
            return a.location.filePath < b.location.filePath;
        }
        if (a.location.line != b.location.line) {
            return a.location.line < b.location.line;
        }
        // Higher severity first
        return static_cast<int>(a.severity) > static_cast<int>(b.severity);
    });

    return sorted;
}

std::vector<std::vector<fusion::Finding>> ReportingEngine::groupBySeverity(
    const std::vector<fusion::Finding>& findings) {

    std::map<fusion::SeverityLevel, std::vector<fusion::Finding>> groups;

    for (const auto& finding : findings) {
        groups[finding.severity].push_back(finding);
    }

    // Return in severity order: CRITICAL, HIGH, MEDIUM, LOW
    std::vector<std::vector<fusion::Finding>> result;

    if (!groups[fusion::SeverityLevel::CRITICAL].empty()) {
        result.push_back(groups[fusion::SeverityLevel::CRITICAL]);
    }
    if (!groups[fusion::SeverityLevel::HIGH].empty()) {
        result.push_back(groups[fusion::SeverityLevel::HIGH]);
    }
    if (!groups[fusion::SeverityLevel::MEDIUM].empty()) {
        result.push_back(groups[fusion::SeverityLevel::MEDIUM]);
    }
    if (!groups[fusion::SeverityLevel::LOW].empty()) {
        result.push_back(groups[fusion::SeverityLevel::LOW]);
    }

    return result;
}

// ============================================================================
// CLI Report Generation
// ============================================================================

std::string ReportingEngine::generateCLIReport(
    const std::vector<fusion::Finding>& findings) const {

    std::ostringstream oss;

    if (findings.empty()) {
        oss << "No implicit conversion hazards detected.\n";
        return oss.str();
    }

    // Header
    oss << "\n";
    oss << "================================================================================\n";
    oss << "  ICHA - IMPLICIT CONVERSION HAZARD ANALYSIS REPORT\n";
    oss << "================================================================================\n";
    oss << "\n";

    // Summary
    oss << formatSummary(findings);
    oss << "\n";

    // Group by severity
    auto groups = groupBySeverity(findings);

    for (const auto& group : groups) {
        if (group.empty()) continue;

        auto severity = group[0].severity;
        oss << "================================================================================\n";
        oss << "  " << formatSeverityCLI(severity) << " SEVERITY FINDINGS\n";
        oss << "================================================================================\n";
        oss << "\n";

        // Sort within group by file and line
        auto sortedGroup = sortFindings(group);

        for (size_t i = 0; i < sortedGroup.size(); ++i) {
            oss << formatFindingCLI(sortedGroup[i]);

            if (i < sortedGroup.size() - 1) {
                oss << "\n";
                oss << "--------------------------------------------------------------------------------\n";
                oss << "\n";
            }
        }

        oss << "\n";
    }

    // Footer
    oss << "================================================================================\n";
    oss << "  END OF REPORT\n";
    oss << "================================================================================\n";

    return oss.str();
}

std::string ReportingEngine::formatSeverityCLI(fusion::SeverityLevel level) const {
    std::string text = fusion::severityToString(level);

    if (!config_.useColors) {
        return text;
    }

    switch (level) {
        case fusion::SeverityLevel::CRITICAL:
            return std::string(COLOR_RED) + text + COLOR_RESET;
        case fusion::SeverityLevel::HIGH:
            return std::string(COLOR_YELLOW) + text + COLOR_RESET;
        case fusion::SeverityLevel::MEDIUM:
            return std::string(COLOR_BLUE) + text + COLOR_RESET;
        case fusion::SeverityLevel::LOW:
            return std::string(COLOR_GREEN) + text + COLOR_RESET;
    }

    return text;
}

std::string ReportingEngine::formatFindingCLI(const fusion::Finding& finding) const {
    std::ostringstream oss;

    // Header
    oss << "[" << formatSeverityCLI(finding.severity) << "] ";
    oss << "Implicit conversion detected\n";
    oss << "\n";

    // Location
    oss << "File:       " << finding.location.filePath << ":" << finding.location.line << "\n";
    oss << "Finding ID: " << finding.id.value << "\n";
    oss << "Conversion: " << finding.sourceType << " → " << finding.targetType << "\n";
    oss << "Category:   " << finding.conversionCategory << "\n";
    oss << "Context:    " << finding.primaryContext << "\n";
    oss << "Risk Score: " << finding.finalRiskScore << "/100\n";
    oss << "\n";

    // Risk breakdown
    oss << "Risk Breakdown:\n";
    oss << "  Conversion Risk:  " << finding.riskBreakdown.conversionRisk << "\n";
    oss << "  Context Risk:     " << finding.riskBreakdown.contextRisk << "\n";
    oss << "  Impact Risk:      " << finding.riskBreakdown.impactRisk << "\n";
    oss << "  Propagation Risk: " << finding.riskBreakdown.propagationRisk << "\n";
    oss << "\n";

    // Impact analysis
    if (!finding.impactCategories.empty()) {
        oss << "Impact Categories:\n";
        for (const auto& category : finding.impactCategories) {
            oss << "  • " << category << "\n";
        }
        oss << "\n";
    }

    // Explanation (if enabled)
    if (config_.includeExplanations && !finding.explanation.empty()) {
        oss << "Explanation:\n";
        // The explanation is already formatted, just indent it
        std::istringstream explanationStream(finding.explanation);
        std::string line;
        while (std::getline(explanationStream, line)) {
            oss << "  " << line << "\n";
        }
        oss << "\n";
    }

    // Suggested fix (if enabled)
    if (config_.includeFixes && !finding.suggestedFix.description.empty()) {
        oss << "Suggested Fix:\n";
        oss << "  " << finding.suggestedFix.description << "\n";

        if (!finding.suggestedFix.steps.empty()) {
            oss << "\n";
            oss << "  Steps:\n";
            for (size_t i = 0; i < finding.suggestedFix.steps.size(); ++i) {
                oss << "    " << (i + 1) << ". " << finding.suggestedFix.steps[i] << "\n";
            }
        }

        if (!finding.suggestedFix.codeSnippet.empty()) {
            oss << "\n";
            oss << "  Code Example:\n";
            std::istringstream snippetStream(finding.suggestedFix.codeSnippet);
            std::string line;
            while (std::getline(snippetStream, line)) {
                oss << "    " << line << "\n";
            }
        }
    }

    return oss.str();
}

std::string ReportingEngine::formatSummary(
    const std::vector<fusion::Finding>& findings) const {

    std::ostringstream oss;

    // Count by severity
    int criticalCount = 0;
    int highCount = 0;
    int mediumCount = 0;
    int lowCount = 0;

    for (const auto& finding : findings) {
        switch (finding.severity) {
            case fusion::SeverityLevel::CRITICAL:
                criticalCount++;
                break;
            case fusion::SeverityLevel::HIGH:
                highCount++;
                break;
            case fusion::SeverityLevel::MEDIUM:
                mediumCount++;
                break;
            case fusion::SeverityLevel::LOW:
                lowCount++;
                break;
        }
    }

    oss << "Summary:\n";
    oss << "  Total Findings: " << findings.size() << "\n";
    oss << "  CRITICAL:       " << criticalCount << "\n";
    oss << "  HIGH:           " << highCount << "\n";
    oss << "  MEDIUM:         " << mediumCount << "\n";
    oss << "  LOW:            " << lowCount << "\n";

    return oss.str();
}

// ============================================================================
// JSON Report Generation
// ============================================================================

std::string ReportingEngine::generateJSONReport(
    const std::vector<fusion::Finding>& findings) const {

    std::ostringstream oss;
    const std::string nl = config_.prettyPrint ? "\n" : "";
    const std::string sp = config_.prettyPrint ? " " : "";

    auto ind = [this](int level) {
        return config_.prettyPrint ? indent(level) : "";
    };

    oss << "{" << nl;
    oss << ind(1) << "\"version\":" << sp << "\"1.0.0\"," << nl;
    oss << ind(1) << "\"tool\":" << sp << "\"ICHA\"," << nl;
    oss << ind(1) << "\"totalFindings\":" << sp << findings.size() << "," << nl;
    oss << ind(1) << "\"findings\":" << sp << "[";

    auto sorted = sortFindings(findings);

    for (size_t i = 0; i < sorted.size(); ++i) {
        if (config_.prettyPrint) {
            oss << nl;
        }
        oss << formatFindingJSON(sorted[i], 2);
        if (i < sorted.size() - 1) {
            oss << ",";
        }
    }

    if (config_.prettyPrint && !sorted.empty()) {
        oss << nl;
    }
    oss << ind(1) << "]" << nl;
    oss << "}";

    return oss.str();
}

std::string ReportingEngine::formatFindingJSON(
    const fusion::Finding& finding, int indentLevel) const {

    std::ostringstream oss;
    const std::string nl = config_.prettyPrint ? "\n" : "";
    const std::string sp = config_.prettyPrint ? " " : "";

    auto ind = [this](int level) {
        return config_.prettyPrint ? indent(level) : "";
    };

    oss << ind(indentLevel) << "{" << nl;
    oss << ind(indentLevel + 1) << "\"id\":" << sp << "\"" << escapeJSON(finding.id.value) << "\"," << nl;

    // Location
    oss << ind(indentLevel + 1) << "\"location\":" << sp << "{" << nl;
    oss << ind(indentLevel + 2) << "\"file\":" << sp << "\"" << escapeJSON(finding.location.filePath) << "\"," << nl;
    oss << ind(indentLevel + 2) << "\"line\":" << sp << finding.location.line << "," << nl;
    oss << ind(indentLevel + 2) << "\"column\":" << sp << finding.location.column << nl;
    oss << ind(indentLevel + 1) << "}," << nl;

    // Conversion
    oss << ind(indentLevel + 1) << "\"conversion\":" << sp << "{" << nl;
    oss << ind(indentLevel + 2) << "\"source\":" << sp << "\"" << escapeJSON(finding.sourceType) << "\"," << nl;
    oss << ind(indentLevel + 2) << "\"target\":" << sp << "\"" << escapeJSON(finding.targetType) << "\"," << nl;
    oss << ind(indentLevel + 2) << "\"category\":" << sp << "\"" << escapeJSON(finding.conversionCategory) << "\"" << nl;
    oss << ind(indentLevel + 1) << "}," << nl;

    // Context
    oss << ind(indentLevel + 1) << "\"context\":" << sp << "\"" << escapeJSON(finding.primaryContext) << "\"," << nl;

    // Risk
    oss << ind(indentLevel + 1) << "\"risk\":" << sp << "{" << nl;
    oss << ind(indentLevel + 2) << "\"conversionRisk\":" << sp << finding.riskBreakdown.conversionRisk << "," << nl;
    oss << ind(indentLevel + 2) << "\"contextRisk\":" << sp << finding.riskBreakdown.contextRisk << "," << nl;
    oss << ind(indentLevel + 2) << "\"impactRisk\":" << sp << finding.riskBreakdown.impactRisk << "," << nl;
    oss << ind(indentLevel + 2) << "\"propagationRisk\":" << sp << finding.riskBreakdown.propagationRisk << "," << nl;
    oss << ind(indentLevel + 2) << "\"finalRisk\":" << sp << finding.finalRiskScore << nl;
    oss << ind(indentLevel + 1) << "}," << nl;

    // Severity
    oss << ind(indentLevel + 1) << "\"severity\":" << sp << "\"" << fusion::severityToString(finding.severity) << "\"," << nl;

    // Impact categories
    oss << ind(indentLevel + 1) << "\"impactCategories\":" << sp << "[";
    for (size_t i = 0; i < finding.impactCategories.size(); ++i) {
        if (i > 0) oss << "," << sp;
        oss << "\"" << escapeJSON(finding.impactCategories[i]) << "\"";
    }
    oss << "]," << nl;

    // Explanation
    if (config_.includeExplanations) {
        oss << ind(indentLevel + 1) << "\"explanation\":" << sp << "\"" << escapeJSON(finding.explanation) << "\"," << nl;
    }

    // Fix
    if (config_.includeFixes) {
        oss << ind(indentLevel + 1) << "\"fix\":" << sp << "{" << nl;
        oss << ind(indentLevel + 2) << "\"description\":" << sp << "\"" << escapeJSON(finding.suggestedFix.description) << "\"," << nl;
        oss << ind(indentLevel + 2) << "\"steps\":" << sp << "[";
        for (size_t i = 0; i < finding.suggestedFix.steps.size(); ++i) {
            if (i > 0) oss << "," << sp;
            oss << "\"" << escapeJSON(finding.suggestedFix.steps[i]) << "\"";
        }
        oss << "]" << nl;
        oss << ind(indentLevel + 1) << "}," << nl;
    }

    // Metadata
    oss << ind(indentLevel + 1) << "\"timestamp\":" << sp << "\"" << escapeJSON(finding.timestamp) << "\"" << nl;

    oss << ind(indentLevel) << "}";

    return oss.str();
}

std::string ReportingEngine::escapeJSON(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            case '\b': oss << "\\b"; break;
            case '\f': oss << "\\f"; break;
            default:   oss << c; break;
        }
    }
    return oss.str();
}

std::string ReportingEngine::indent(int level) {
    return std::string(level * 2, ' ');
}

// ============================================================================
// SARIF Report Generation
// ============================================================================

std::string ReportingEngine::generateSARIFReport(
    const std::vector<fusion::Finding>& findings) const {

    std::ostringstream oss;
    const std::string nl = "\n";

    auto ind = [](int level) { return std::string(level * 2, ' '); };

    oss << "{" << nl;
    oss << ind(1) << "\"version\": \"2.1.0\"," << nl;
    oss << ind(1) << "\"$schema\": \"https://raw.githubusercontent.com/oasis-tcs/sarif-spec/master/Schemata/sarif-schema-2.1.0.json\"," << nl;
    oss << ind(1) << "\"runs\": [" << nl;
    oss << ind(2) << "{" << nl;

    // Tool information
    oss << ind(3) << "\"tool\": {" << nl;
    oss << ind(4) << "\"driver\": {" << nl;
    oss << ind(5) << "\"name\": \"ICHA\"," << nl;
    oss << ind(5) << "\"fullName\": \"Implicit Conversion Hazard Analyzer\"," << nl;
    oss << ind(5) << "\"version\": \"1.0.0\"," << nl;
    oss << ind(5) << "\"informationUri\": \"https://github.com/icha/icha\"," << nl;
    oss << ind(5) << "\"rules\": [" << nl;
    oss << generateSARIFRules();
    oss << ind(5) << "]" << nl;
    oss << ind(4) << "}" << nl;
    oss << ind(3) << "}," << nl;

    // Results
    oss << ind(3) << "\"results\": [";

    auto sorted = sortFindings(findings);

    for (size_t i = 0; i < sorted.size(); ++i) {
        oss << nl;
        oss << formatFindingSARIF(sorted[i], 4);
        if (i < sorted.size() - 1) {
            oss << ",";
        }
    }

    if (!sorted.empty()) {
        oss << nl;
    }
    oss << ind(3) << "]" << nl;

    oss << ind(2) << "}" << nl;
    oss << ind(1) << "]" << nl;
    oss << "}";

    return oss.str();
}

std::string ReportingEngine::generateSARIFRules() const {
    std::ostringstream oss;
    const std::string nl = "\n";

    auto ind = [](int level) { return std::string(level * 2, ' '); };

    // Define rules for each conversion category
    std::vector<std::pair<std::string, std::string>> rules = {
        {"ICHA-Narrowing", "Narrowing conversion detected"},
        {"ICHA-Widening", "Widening conversion detected"},
        {"ICHA-SignChange", "Sign change conversion detected"},
        {"ICHA-EnumConversion", "Enum conversion detected"},
        {"ICHA-BooleanConversion", "Boolean conversion detected"},
        {"ICHA-PointerConversion", "Pointer conversion detected"}
    };

    for (size_t i = 0; i < rules.size(); ++i) {
        oss << ind(6) << "{" << nl;
        oss << ind(7) << "\"id\": \"" << rules[i].first << "\"," << nl;
        oss << ind(7) << "\"name\": \"" << rules[i].first << "\"," << nl;
        oss << ind(7) << "\"shortDescription\": {" << nl;
        oss << ind(8) << "\"text\": \"" << rules[i].second << "\"" << nl;
        oss << ind(7) << "}," << nl;
        oss << ind(7) << "\"fullDescription\": {" << nl;
        oss << ind(8) << "\"text\": \"Implicit type conversion that may introduce defects, security vulnerabilities, or maintenance risks.\"" << nl;
        oss << ind(7) << "}," << nl;
        oss << ind(7) << "\"defaultConfiguration\": {" << nl;
        oss << ind(8) << "\"level\": \"warning\"" << nl;
        oss << ind(7) << "}" << nl;
        oss << ind(6) << "}";

        if (i < rules.size() - 1) {
            oss << ",";
        }
        oss << nl;
    }

    return oss.str();
}

std::string ReportingEngine::formatFindingSARIF(
    const fusion::Finding& finding, int indentLevel) const {

    std::ostringstream oss;
    const std::string nl = "\n";

    auto ind = [](int level) { return std::string(level * 2, ' '); };

    oss << ind(indentLevel) << "{" << nl;
    oss << ind(indentLevel + 1) << "\"ruleId\": \"ICHA-" << finding.conversionCategory << "\"," << nl;
    oss << ind(indentLevel + 1) << "\"level\": \"" << mapSeverityToSARIF(finding.severity) << "\"," << nl;
    oss << ind(indentLevel + 1) << "\"message\": {" << nl;
    oss << ind(indentLevel + 2) << "\"text\": \"" << escapeJSON(
        "Implicit conversion from '" + finding.sourceType + "' to '" + finding.targetType +
        "' detected. Risk score: " + std::to_string(finding.finalRiskScore) + "/100"
    ) << "\"" << nl;
    oss << ind(indentLevel + 1) << "}," << nl;

    // Locations
    oss << ind(indentLevel + 1) << "\"locations\": [{" << nl;
    oss << ind(indentLevel + 2) << "\"physicalLocation\": {" << nl;
    oss << ind(indentLevel + 3) << "\"artifactLocation\": {" << nl;
    oss << ind(indentLevel + 4) << "\"uri\": \"" << escapeJSON(finding.location.filePath) << "\"" << nl;
    oss << ind(indentLevel + 3) << "}," << nl;
    oss << ind(indentLevel + 3) << "\"region\": {" << nl;
    oss << ind(indentLevel + 4) << "\"startLine\": " << finding.location.line << "," << nl;
    oss << ind(indentLevel + 4) << "\"startColumn\": " << finding.location.column << nl;
    oss << ind(indentLevel + 3) << "}" << nl;
    oss << ind(indentLevel + 2) << "}" << nl;
    oss << ind(indentLevel + 1) << "}]," << nl;

    // Properties (additional metadata)
    oss << ind(indentLevel + 1) << "\"properties\": {" << nl;
    oss << ind(indentLevel + 2) << "\"findingId\": \"" << escapeJSON(finding.id.value) << "\"," << nl;
    oss << ind(indentLevel + 2) << "\"riskScore\": " << finding.finalRiskScore << "," << nl;
    oss << ind(indentLevel + 2) << "\"severity\": \"" << fusion::severityToString(finding.severity) << "\"," << nl;
    oss << ind(indentLevel + 2) << "\"context\": \"" << escapeJSON(finding.primaryContext) << "\"," << nl;
    oss << ind(indentLevel + 2) << "\"conversionCategory\": \"" << escapeJSON(finding.conversionCategory) << "\"," << nl;
    oss << ind(indentLevel + 2) << "\"impactCategories\": [";

    for (size_t i = 0; i < finding.impactCategories.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << "\"" << escapeJSON(finding.impactCategories[i]) << "\"";
    }

    oss << "]," << nl;
    oss << ind(indentLevel + 2) << "\"riskBreakdown\": {" << nl;
    oss << ind(indentLevel + 3) << "\"conversionRisk\": " << finding.riskBreakdown.conversionRisk << "," << nl;
    oss << ind(indentLevel + 3) << "\"contextRisk\": " << finding.riskBreakdown.contextRisk << "," << nl;
    oss << ind(indentLevel + 3) << "\"impactRisk\": " << finding.riskBreakdown.impactRisk << "," << nl;
    oss << ind(indentLevel + 3) << "\"propagationRisk\": " << finding.riskBreakdown.propagationRisk << nl;
    oss << ind(indentLevel + 2) << "}" << nl;
    oss << ind(indentLevel + 1) << "}" << nl;

    oss << ind(indentLevel) << "}";

    return oss.str();
}

std::string ReportingEngine::mapSeverityToSARIF(fusion::SeverityLevel level) {
    switch (level) {
        case fusion::SeverityLevel::LOW:
            return "note";
        case fusion::SeverityLevel::MEDIUM:
            return "warning";
        case fusion::SeverityLevel::HIGH:
            return "error";
        case fusion::SeverityLevel::CRITICAL:
            return "error";
    }
    return "warning";
}

}  // namespace reporting
