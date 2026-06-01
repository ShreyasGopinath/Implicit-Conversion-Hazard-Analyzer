#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace fusion {

enum class SeverityLevel {
    LOW,
    MEDIUM,
    HIGH,
    CRITICAL
};

struct FindingID {
    std::string value;

    static FindingID generate(const std::string& filePath, unsigned line, unsigned column);
};

struct FileLocation {
    std::string filePath;
    unsigned line{ 0 };
    unsigned column{ 0 };

    std::string toString() const;
};

struct RiskBreakdown {
    std::uint32_t conversionRisk{ 0 };
    std::uint32_t contextRisk{ 0 };
    std::uint32_t impactRisk{ 0 };
    std::uint32_t propagationRisk{ 0 };

    std::uint32_t total() const {
        return conversionRisk + contextRisk + impactRisk + propagationRisk;
    }
};

struct SuggestedFix {
    std::string description;
    std::string codeSnippet;
    std::vector<std::string> steps;
};

struct Finding {
    FindingID id;
    FileLocation location;

    // Conversion details
    std::string sourceType;
    std::string targetType;
    std::string conversionCategory;

    // Context information
    std::string primaryContext;
    std::vector<std::string> secondaryContexts;
    float contextConfidence{ 0.0f };

    // Impact information
    std::vector<std::string> impactCategories;
    std::vector<std::string> impactPaths;
    bool reachesMemoryAccess{ false };
    bool reachesControlFlow{ false };
    bool reachesAPIBoundary{ false };

    // Risk assessment
    RiskBreakdown riskBreakdown;
    std::uint32_t finalRiskScore{ 0 };
    SeverityLevel severity{ SeverityLevel::LOW };

    // Explanation and fix
    std::string explanation;
    SuggestedFix suggestedFix;

    // Metadata
    std::string timestamp;
    std::string analyzerVersion;
};

inline std::string severityToString(SeverityLevel severity) {
    switch (severity) {
        case SeverityLevel::LOW:
            return "LOW";
        case SeverityLevel::MEDIUM:
            return "MEDIUM";
        case SeverityLevel::HIGH:
            return "HIGH";
        case SeverityLevel::CRITICAL:
            return "CRITICAL";
    }
    return "LOW";
}

}  // namespace fusion
