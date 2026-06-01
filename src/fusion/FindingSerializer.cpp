#include "fusion/FindingSerializer.hpp"

#include <sstream>

namespace fusion {

std::string FindingSerializer::escapeJSON(const std::string& str) {
    std::ostringstream oss;
    for (char c : str) {
        switch (c) {
            case '"':  oss << "\\\""; break;
            case '\\': oss << "\\\\"; break;
            case '\n': oss << "\\n"; break;
            case '\r': oss << "\\r"; break;
            case '\t': oss << "\\t"; break;
            default:   oss << c; break;
        }
    }
    return oss.str();
}

std::string FindingSerializer::indent(int level) {
    return std::string(level * 2, ' ');
}

std::string FindingSerializer::toJSON(const Finding& finding, bool pretty) {
    std::ostringstream oss;
    const std::string nl = pretty ? "\n" : "";
    const std::string sp = pretty ? " " : "";

    auto ind = [&](int level) { return pretty ? indent(level) : ""; };

    oss << "{" << nl;
    oss << ind(1) << "\"id\":" << sp << "\"" << escapeJSON(finding.id.value) << "\"," << nl;
    oss << ind(1) << "\"location\":" << sp << "{" << nl;
    oss << ind(2) << "\"file\":" << sp << "\"" << escapeJSON(finding.location.filePath) << "\"," << nl;
    oss << ind(2) << "\"line\":" << sp << finding.location.line << "," << nl;
    oss << ind(2) << "\"column\":" << sp << finding.location.column << nl;
    oss << ind(1) << "}," << nl;

    oss << ind(1) << "\"conversion\":" << sp << "{" << nl;
    oss << ind(2) << "\"source\":" << sp << "\"" << escapeJSON(finding.sourceType) << "\"," << nl;
    oss << ind(2) << "\"target\":" << sp << "\"" << escapeJSON(finding.targetType) << "\"," << nl;
    oss << ind(2) << "\"category\":" << sp << "\"" << escapeJSON(finding.conversionCategory) << "\"" << nl;
    oss << ind(1) << "}," << nl;

    oss << ind(1) << "\"context\":" << sp << "{" << nl;
    oss << ind(2) << "\"primary\":" << sp << "\"" << escapeJSON(finding.primaryContext) << "\"," << nl;
    oss << ind(2) << "\"secondary\":" << sp << "[";
    for (size_t i = 0; i < finding.secondaryContexts.size(); ++i) {
        if (i > 0) oss << "," << sp;
        oss << "\"" << escapeJSON(finding.secondaryContexts[i]) << "\"";
    }
    oss << "]," << nl;
    oss << ind(2) << "\"confidence\":" << sp << finding.contextConfidence << nl;
    oss << ind(1) << "}," << nl;

    oss << ind(1) << "\"impact\":" << sp << "{" << nl;
    oss << ind(2) << "\"categories\":" << sp << "[";
    for (size_t i = 0; i < finding.impactCategories.size(); ++i) {
        if (i > 0) oss << "," << sp;
        oss << "\"" << escapeJSON(finding.impactCategories[i]) << "\"";
    }
    oss << "]," << nl;
    oss << ind(2) << "\"reachesMemoryAccess\":" << sp << (finding.reachesMemoryAccess ? "true" : "false") << "," << nl;
    oss << ind(2) << "\"reachesControlFlow\":" << sp << (finding.reachesControlFlow ? "true" : "false") << "," << nl;
    oss << ind(2) << "\"reachesAPIBoundary\":" << sp << (finding.reachesAPIBoundary ? "true" : "false") << "," << nl;
    oss << ind(2) << "\"pathCount\":" << sp << finding.impactPaths.size() << nl;
    oss << ind(1) << "}," << nl;

    oss << ind(1) << "\"risk\":" << sp << "{" << nl;
    oss << ind(2) << "\"breakdown\":" << sp << "{" << nl;
    oss << ind(3) << "\"conversion\":" << sp << finding.riskBreakdown.conversionRisk << "," << nl;
    oss << ind(3) << "\"context\":" << sp << finding.riskBreakdown.contextRisk << "," << nl;
    oss << ind(3) << "\"impact\":" << sp << finding.riskBreakdown.impactRisk << "," << nl;
    oss << ind(3) << "\"propagation\":" << sp << finding.riskBreakdown.propagationRisk << nl;
    oss << ind(2) << "}," << nl;
    oss << ind(2) << "\"finalScore\":" << sp << finding.finalRiskScore << "," << nl;
    oss << ind(2) << "\"severity\":" << sp << "\"" << severityToString(finding.severity) << "\"" << nl;
    oss << ind(1) << "}," << nl;

    oss << ind(1) << "\"explanation\":" << sp << "\"" << escapeJSON(finding.explanation) << "\"," << nl;

    oss << ind(1) << "\"suggestedFix\":" << sp << "{" << nl;
    oss << ind(2) << "\"description\":" << sp << "\"" << escapeJSON(finding.suggestedFix.description) << "\"," << nl;
    oss << ind(2) << "\"codeSnippet\":" << sp << "\"" << escapeJSON(finding.suggestedFix.codeSnippet) << "\"," << nl;
    oss << ind(2) << "\"steps\":" << sp << "[";
    for (size_t i = 0; i < finding.suggestedFix.steps.size(); ++i) {
        if (i > 0) oss << "," << sp;
        oss << "\"" << escapeJSON(finding.suggestedFix.steps[i]) << "\"";
    }
    oss << "]" << nl;
    oss << ind(1) << "}," << nl;

    oss << ind(1) << "\"metadata\":" << sp << "{" << nl;
    oss << ind(2) << "\"timestamp\":" << sp << "\"" << escapeJSON(finding.timestamp) << "\"," << nl;
    oss << ind(2) << "\"analyzerVersion\":" << sp << "\"" << escapeJSON(finding.analyzerVersion) << "\"" << nl;
    oss << ind(1) << "}" << nl;

    oss << "}";

    return oss.str();
}

std::string FindingSerializer::toSARIF(const Finding& finding) {
    std::ostringstream oss;

    oss << "{\n";
    oss << "  \"ruleId\": \"ICHA-" << finding.conversionCategory << "\",\n";
    oss << "  \"level\": \"";

    switch (finding.severity) {
        case SeverityLevel::LOW:
            oss << "note";
            break;
        case SeverityLevel::MEDIUM:
            oss << "warning";
            break;
        case SeverityLevel::HIGH:
            oss << "error";
            break;
        case SeverityLevel::CRITICAL:
            oss << "error";
            break;
    }

    oss << "\",\n";
    oss << "  \"message\": {\n";
    oss << "    \"text\": \"" << escapeJSON(finding.explanation) << "\"\n";
    oss << "  },\n";
    oss << "  \"locations\": [{\n";
    oss << "    \"physicalLocation\": {\n";
    oss << "      \"artifactLocation\": {\n";
    oss << "        \"uri\": \"" << escapeJSON(finding.location.filePath) << "\"\n";
    oss << "      },\n";
    oss << "      \"region\": {\n";
    oss << "        \"startLine\": " << finding.location.line << ",\n";
    oss << "        \"startColumn\": " << finding.location.column << "\n";
    oss << "      }\n";
    oss << "    }\n";
    oss << "  }],\n";
    oss << "  \"properties\": {\n";
    oss << "    \"riskScore\": " << finding.finalRiskScore << ",\n";
    oss << "    \"severity\": \"" << severityToString(finding.severity) << "\",\n";
    oss << "    \"context\": \"" << escapeJSON(finding.primaryContext) << "\",\n";
    oss << "    \"impactCategories\": [";

    for (size_t i = 0; i < finding.impactCategories.size(); ++i) {
        if (i > 0) oss << ", ";
        oss << "\"" << escapeJSON(finding.impactCategories[i]) << "\"";
    }

    oss << "]\n";
    oss << "  }\n";
    oss << "}";

    return oss.str();
}

}  // namespace fusion
