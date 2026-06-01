/**
 * Risk Intelligence Engine Demonstration
 *
 * This example demonstrates the deterministic risk scoring engine
 * for the Implicit Conversion Hazard Analyzer (ICHA).
 */

#include <iostream>
#include <iomanip>

#include "context/ContextRecord.hpp"
#include "domain/conversion/ConversionRecord.hpp"
#include "risk/RiskEngine.hpp"

void printSeparator() {
    std::cout << std::string(80, '=') << '\n';
}

void printRiskAssessment(const std::string& scenario, const risk::RiskAssessment& assessment) {
    printSeparator();
    std::cout << "SCENARIO: " << scenario << '\n';
    printSeparator();

    std::cout << "\nConversion Details:\n";
    std::cout << "  Source Type: " << assessment.conversion.sourceType << '\n';
    std::cout << "  Target Type: " << assessment.conversion.targetType << '\n';

    std::cout << "\nRisk Breakdown:\n";
    std::cout << "  Conversion Risk:  " << std::setw(3) << assessment.breakdown.conversionRisk << '\n';
    std::cout << "  Context Risk:     " << std::setw(3) << assessment.breakdown.contextRisk << '\n';
    std::cout << "  Impact Risk:      " << std::setw(3) << assessment.breakdown.impactRisk << '\n';
    std::cout << "  Propagation Risk: " << std::setw(3) << assessment.breakdown.propagationRisk << '\n';
    std::cout << "  ─────────────────────\n";
    std::cout << "  TOTAL SCORE:      " << std::setw(3) << assessment.breakdown.total() << '\n';

    std::cout << "\nSeverity: ";
    switch (assessment.severity) {
        case risk::Severity::Low:
            std::cout << "LOW\n";
            break;
        case risk::Severity::Medium:
            std::cout << "MEDIUM\n";
            break;
        case risk::Severity::High:
            std::cout << "HIGH\n";
            break;
        case risk::Severity::Critical:
            std::cout << "CRITICAL\n";
            break;
    }

    std::cout << "\nExplanation:\n  " << assessment.explanation << '\n';
    std::cout << "\nSuggested Fix:\n  " << assessment.suggestedFix << '\n';
    std::cout << '\n';
}

int main() {
    auto engine = risk::makeRiskEngine();

    std::cout << "\n";
    printSeparator();
    std::cout << "  IMPLICIT CONVERSION HAZARD ANALYZER - RISK ENGINE DEMO\n";
    printSeparator();
    std::cout << "\n";

    // Example 1: Critical - Signed to Unsigned in Array Indexing
    {
        domain::conversion::ConversionRecord conversion{};
        conversion.sourceType = "int";
        conversion.targetType = "unsigned int";
        conversion.sourceIsSigned = true;
        conversion.targetIsSigned = false;

        context::ContextRecord context{};
        context.inArrayIndex = true;

        risk::ImpactProfile impact{};
        impact.affectsMemoryAccess = true;

        auto assessment = engine->assess(
            conversion, context, impact, risk::PropagationLevel::ImmediateUse);

        printRiskAssessment("Signed to Unsigned in Array Indexing", assessment);
    }

    // Example 2: Critical - Pointer to Integer at API Boundary
    {
        domain::conversion::ConversionRecord conversion{};
        conversion.sourceType = "void *";
        conversion.targetType = "long";
        conversion.sourceIsPointer = true;

        context::ContextRecord context{};
        context.inFunctionArgument = true;
        context.inApiBoundary = true;

        risk::ImpactProfile impact{};
        impact.affectsArithmetic = true;

        auto assessment = engine->assess(
            conversion, context, impact, risk::PropagationLevel::ApiBoundary);

        printRiskAssessment("Pointer to Integer at API Boundary", assessment);
    }

    // Example 3: High - Float to Int in Loop Condition
    {
        domain::conversion::ConversionRecord conversion{};
        conversion.sourceType = "float";
        conversion.targetType = "int";

        context::ContextRecord context{};
        context.inLoopCondition = true;

        risk::ImpactProfile impact{};
        impact.affectsControlFlow = true;

        auto assessment = engine->assess(
            conversion, context, impact, risk::PropagationLevel::SameExpressionChain);

        printRiskAssessment("Float to Int in Loop Condition", assessment);
    }

    // Example 4: Medium - Double to Int in Return Statement
    {
        domain::conversion::ConversionRecord conversion{};
        conversion.sourceType = "double";
        conversion.targetType = "int";

        context::ContextRecord context{};
        context.inReturnStatement = true;

        risk::ImpactProfile impact{};
        impact.affectsArithmetic = true;

        auto assessment = engine->assess(
            conversion, context, impact, risk::PropagationLevel::None);

        printRiskAssessment("Double to Int in Return Statement", assessment);
    }

    // Example 5: Low - Enum to Int in Assignment
    {
        domain::conversion::ConversionRecord conversion{};
        conversion.sourceType = "enum Color";
        conversion.targetType = "int";
        conversion.sourceIsEnum = true;

        context::ContextRecord context{};
        context.isAssignmentOnly = true;

        risk::ImpactProfile impact{};

        auto assessment = engine->assess(
            conversion, context, impact, risk::PropagationLevel::None);

        printRiskAssessment("Enum to Int in Simple Assignment", assessment);
    }

    printSeparator();
    std::cout << "Demo completed successfully.\n";
    printSeparator();
    std::cout << "\n";

    return 0;
}
