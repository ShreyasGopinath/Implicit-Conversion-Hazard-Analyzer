/**
 * ICHA - Implicit Conversion Hazard Analyzer
 *
 * Main entry point for the integrated static analysis tool
 */

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"

#include "ICHAAction.hpp"
#include "fusion/FusionEngine.hpp"
#include "reporting/ReportingEngine.hpp"

using namespace clang::tooling;
using namespace llvm;

// Command line options
static cl::OptionCategory ICHACategory("ICHA Options");

static cl::opt<std::string> OutputFormat(
    "icha-format",
    cl::desc("Output format: cli, json, sarif"),
    cl::value_desc("format"),
    cl::init("cli"),
    cl::cat(ICHACategory)
);

static cl::opt<std::string> OutputFile(
    "icha-output",
    cl::desc("Output file path (default: stdout)"),
    cl::value_desc("filename"),
    cl::init(""),
    cl::cat(ICHACategory)
);

static cl::opt<bool> UseColors(
    "icha-color",
    cl::desc("Use colors in CLI output"),
    cl::init(true),
    cl::cat(ICHACategory)
);

static cl::opt<bool> IncludeExplanations(
    "icha-explain",
    cl::desc("Include detailed explanations"),
    cl::init(true),
    cl::cat(ICHACategory)
);

static cl::opt<bool> IncludeFixes(
    "icha-fixes",
    cl::desc("Include fix suggestions"),
    cl::init(true),
    cl::cat(ICHACategory)
);

static cl::opt<int> MinimumSeverity(
    "icha-min-severity",
    cl::desc("Minimum severity level (0=LOW, 1=MEDIUM, 2=HIGH, 3=CRITICAL)"),
    cl::value_desc("level"),
    cl::init(0),
    cl::cat(ICHACategory)
);

static cl::opt<bool> Verbose(
    "icha-verbose",
    cl::desc("Enable verbose output"),
    cl::init(false),
    cl::cat(ICHACategory)
);

static cl::opt<bool> DebugContext(
    "icha-debug-context",
    cl::desc("Print AST parent chain for each conversion (debug mode)"),
    cl::init(false),
    cl::cat(ICHACategory)
);

int main(int argc, const char **argv) {
    // Parse command line options
    auto expectedParser = CommonOptionsParser::create(
        argc, argv, ICHACategory,
        cl::ZeroOrMore,
        "ICHA - Implicit Conversion Hazard Analyzer\n\n"
        "Analyzes C/C++ code for dangerous implicit type conversions.\n"
    );

    if (!expectedParser) {
        llvm::errs() << expectedParser.takeError();
        return 1;
    }

    CommonOptionsParser& optionsParser = expectedParser.get();

    // Get source files
    auto sourceFiles = optionsParser.getSourcePathList();
    if (sourceFiles.empty()) {
        llvm::errs() << "Error: No source files specified\n";
        return 1;
    }

    if (Verbose) {
        std::cout << "ICHA - Implicit Conversion Hazard Analyzer\n";
        std::cout << "Analyzing " << sourceFiles.size() << " file(s)...\n\n";
    }

    // Initialize components
    auto fusionEngine = fusion::makeFusionEngine();
    std::vector<fusion::Finding> allFindings;

    // Create tool and run analysis
    ClangTool tool(optionsParser.getCompilations(), sourceFiles);

    auto actionFactory = std::make_unique<icha::ICHAActionFactory>(
        allFindings, fusionEngine.get(), DebugContext);

    int result = tool.run(actionFactory.get());

    if (result != 0) {
        llvm::errs() << "Error: Analysis failed\n";
        return result;
    }

    if (Verbose) {
        std::cout << "\nFound " << allFindings.size() << " conversions\n";
    }

    // Filter by severity
    std::vector<fusion::Finding> filtered;
    fusion::SeverityLevel minLevel;
    switch (MinimumSeverity) {
        case 0: minLevel = fusion::SeverityLevel::LOW; break;
        case 1: minLevel = fusion::SeverityLevel::MEDIUM; break;
        case 2: minLevel = fusion::SeverityLevel::HIGH; break;
        case 3: minLevel = fusion::SeverityLevel::CRITICAL; break;
        default: minLevel = fusion::SeverityLevel::LOW;
    }

    for (const auto& finding : allFindings) {
        if (static_cast<int>(finding.severity) >= static_cast<int>(minLevel)) {
            filtered.push_back(finding);
        }
    }

    if (Verbose) {
        std::cout << "After filtering: " << filtered.size() << " findings\n\n";
    }

    // Generate report
    reporting::ReportConfig reportConfig;
    reportConfig.useColors = UseColors;
    reportConfig.includeExplanations = IncludeExplanations;
    reportConfig.includeFixes = IncludeFixes;

    auto reportingEngine = std::make_unique<reporting::ReportingEngine>(reportConfig);

    std::string report;
    if (OutputFormat == "json") {
        report = reportingEngine->generateJSONReport(filtered);
    } else if (OutputFormat == "sarif") {
        report = reportingEngine->generateSARIFReport(filtered);
    } else {
        report = reportingEngine->generateCLIReport(filtered);
    }

    // Output to file or stdout
    if (OutputFile.empty()) {
        std::cout << report;
    } else {
        std::ofstream outFile(OutputFile);
        if (outFile.is_open()) {
            outFile << report;
            outFile.close();
            if (Verbose) {
                std::cout << "Report written to: " << OutputFile << "\n";
            }
        } else {
            std::cerr << "Error: Could not write to file: " << OutputFile << "\n";
            return 1;
        }
    }

    return filtered.empty() ? 0 : 1;
}
