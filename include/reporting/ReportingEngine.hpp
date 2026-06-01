#pragma once

#include <string>
#include <vector>

#include "fusion/Finding.hpp"

namespace reporting {

// Configuration for report generation
struct ReportConfig {
    bool useColors{false};           // Enable ANSI color codes in CLI output
    bool prettyPrint{true};          // Pretty-print JSON output
    bool includeExplanations{true};  // Include detailed explanations
    bool includeFixes{true};         // Include suggested fixes
};

class ReportingEngine {
   public:
    explicit ReportingEngine(const ReportConfig& config = ReportConfig{});

    // Generate CLI report (human-readable)
    std::string generateCLIReport(const std::vector<fusion::Finding>& findings) const;

    // Generate JSON report (machine-readable)
    std::string generateJSONReport(const std::vector<fusion::Finding>& findings) const;

    // Generate SARIF 2.1.0 compliant report
    std::string generateSARIFReport(const std::vector<fusion::Finding>& findings) const;

   private:
    ReportConfig config_;

    // Sorting and grouping
    static std::vector<fusion::Finding> sortFindings(const std::vector<fusion::Finding>& findings);
    static std::vector<std::vector<fusion::Finding>> groupBySeverity(
        const std::vector<fusion::Finding>& findings);

    // CLI formatting helpers
    std::string formatSeverityCLI(fusion::SeverityLevel level) const;
    std::string formatFindingCLI(const fusion::Finding& finding) const;
    std::string formatSummary(const std::vector<fusion::Finding>& findings) const;

    // JSON formatting helpers
    std::string formatFindingJSON(const fusion::Finding& finding, int indent) const;
    static std::string escapeJSON(const std::string& str);
    static std::string indent(int level);

    // SARIF formatting helpers
    std::string generateSARIFHeader() const;
    std::string generateSARIFRules() const;
    std::string formatFindingSARIF(const fusion::Finding& finding, int indent) const;
    static std::string mapSeverityToSARIF(fusion::SeverityLevel level);

    // Color codes (ANSI)
    static constexpr const char* COLOR_RESET = "\033[0m";
    static constexpr const char* COLOR_RED = "\033[1;31m";
    static constexpr const char* COLOR_YELLOW = "\033[1;33m";
    static constexpr const char* COLOR_BLUE = "\033[1;34m";
    static constexpr const char* COLOR_GREEN = "\033[1;32m";
    static constexpr const char* COLOR_BOLD = "\033[1m";
};

}  // namespace reporting
