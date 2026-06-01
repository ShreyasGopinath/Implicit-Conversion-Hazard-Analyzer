/**
 * ICHA Analysis Action
 *
 * Clang FrontendAction that runs ICHA analysis using the visitor
 */

#pragma once

#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"

#include "parsing/detail/ImplicitCastVisitor.hpp"
#include "context/EnhancedContextIntelligenceEngine.hpp"
#include "fusion/Finding.hpp"
#include "fusion/FusionEngine.hpp"

#include <vector>
#include <memory>

namespace icha {

/**
 * AST Consumer that uses the ImplicitCastVisitor
 */
class ICHAConsumer : public clang::ASTConsumer {
public:
    explicit ICHAConsumer(clang::ASTContext* context,
                         std::vector<fusion::Finding>& findings,
                         fusion::FusionEngine* fusionEngine,
                         bool debugContext = false)
        : context_(context), findings_(findings), fusionEngine_(fusionEngine),
          debugContext_(debugContext) {

        // Initialize context engine for proper classification
        contextEngine_ = context::makeEnhancedContextIntelligenceEngine();
    }

    void HandleTranslationUnit(clang::ASTContext& context) override {
        // Collect all implicit cast events using the visitor
        std::vector<parsing::ImplicitCastEvent> events;

        auto visitor = parsing::detail::ImplicitCastVisitor(
            [&events](const parsing::ImplicitCastEvent& event) {
                events.push_back(event);
            },
            context
        );

        // Traverse the AST
        visitor.TraverseDecl(context.getTranslationUnitDecl());

        // Process each conversion
        for (const auto& event : events) {
            // Debug mode: print AST parent chain
            if (debugContext_) {
                printASTParentChain(event, context);
            }

            // Use context engine to classify the context
            context::EnhancedContextRecord contextRecord;
            contextEngine_->analyze(event, [&contextRecord](const context::EnhancedContextRecord& record) {
                contextRecord = record;
            });

            // Create a finding with proper context classification
            auto finding = createFinding(event, contextRecord);

            // Only include findings that meet minimum severity threshold
            if (finding.finalRiskScore >= 30) {  // MEDIUM or higher
                findings_.push_back(finding);
            }
        }
    }

private:
    void printASTParentChain(const parsing::ImplicitCastEvent& event, clang::ASTContext& context) {
        std::cout << "\n========================================\n";
        std::cout << "DEBUG: AST Parent Chain\n";
        std::cout << "========================================\n";
        std::cout << "Location: " << event.location.filePath << ":"
                  << event.location.line << ":" << event.location.column << "\n";
        std::cout << "Conversion: " << event.sourceTypeName << " -> " << event.targetTypeName << "\n";
        std::cout << "\nAST Parent Chain:\n";
        std::cout << "  ImplicitCastExpr\n";

        for (const auto& nodeName : event.metadata.parentNodeChain) {
            std::cout << "  -> " << nodeName << "\n";
        }

        // Print detected contexts
        std::cout << "\nDetected Contexts:\n";
        if (event.metadata.inLoopCondition) {
            std::cout << "  ✓ LOOP_CONTEXT (ForStmt/WhileStmt/DoStmt)\n";
        }
        if (event.metadata.inBranchCondition) {
            std::cout << "  ✓ BRANCH_CONTEXT (IfStmt/SwitchStmt)\n";
        }
        if (event.metadata.inArraySubscript) {
            std::cout << "  ✓ MEMORY_CONTEXT (ArraySubscriptExpr)\n";
        }
        if (event.metadata.inReturnStatement) {
            std::cout << "  ✓ RETURN_CONTEXT (ReturnStmt)\n";
        }
        if (event.metadata.inApiBoundary) {
            std::cout << "  ✓ API_CONTEXT (CallExpr)\n";
        }
        if (event.metadata.inFunctionArgument) {
            std::cout << "  ✓ FUNCTION_ARGUMENT (CallExpr argument)\n";
        }
        if (event.metadata.inMemoryAllocation) {
            std::cout << "  ✓ MEMORY_ALLOCATION (CXXNewExpr/CXXDeleteExpr)\n";
        }
        if (event.metadata.inArithmeticExpression) {
            std::cout << "  ✓ ARITHMETIC_CONTEXT (BinaryOperator/UnaryOperator)\n";
        }

        if (!event.metadata.inLoopCondition && !event.metadata.inBranchCondition &&
            !event.metadata.inArraySubscript && !event.metadata.inReturnStatement &&
            !event.metadata.inApiBoundary && !event.metadata.inMemoryAllocation) {
            std::cout << "  ✓ ASSIGNMENT_CONTEXT (default)\n";
        }

        std::cout << "========================================\n\n";
    }
    fusion::Finding createFinding(const parsing::ImplicitCastEvent& event,
                                  const context::EnhancedContextRecord& contextRecord) {
        fusion::Finding finding;

        // Location
        finding.location.filePath = event.location.filePath;
        finding.location.line = event.location.line;
        finding.location.column = event.location.column;

        finding.id = fusion::FindingID::generate(
            finding.location.filePath,
            finding.location.line,
            finding.location.column
        );

        // Types
        finding.sourceType = event.sourceTypeName;
        finding.targetType = event.targetTypeName;

        // Category
        finding.conversionCategory = determineCategoryFromCastKind(event.castKind);

        // Context - USE THE ACTUAL DETECTED CONTEXT
        finding.primaryContext = context::contextTypeToString(contextRecord.classification.primaryContext);
        finding.contextConfidence = contextRecord.classification.confidenceScore;

        // Risk scoring based on context
        finding.riskBreakdown = calculateRiskBreakdown(event, contextRecord);
        finding.finalRiskScore = finding.riskBreakdown.conversionRisk +
                                finding.riskBreakdown.contextRisk +
                                finding.riskBreakdown.impactRisk +
                                finding.riskBreakdown.propagationRisk;

        // Severity
        finding.severity = computeSeverity(finding.finalRiskScore);

        // Explanation
        finding.explanation = contextRecord.classification.semanticExplanation;
        if (finding.explanation.empty()) {
            finding.explanation = "Implicit conversion from '" + finding.sourceType +
                                "' to '" + finding.targetType + "' detected.";
        }

        // Fix suggestion
        finding.suggestedFix = generateFixSuggestion(event, contextRecord);

        finding.timestamp = "2026-06-01";
        finding.analyzerVersion = "ICHA 1.0.0";

        return finding;
    }

    fusion::RiskBreakdown calculateRiskBreakdown(const parsing::ImplicitCastEvent& event,
                                                 const context::EnhancedContextRecord& contextRecord) {
        fusion::RiskBreakdown breakdown;

        // Conversion risk based on cast kind
        switch (event.castKind) {
            case parsing::CastKind::IntegralCast:
                breakdown.conversionRisk = 25;
                break;
            case parsing::CastKind::FloatingToIntegral:
                breakdown.conversionRisk = 25;
                break;
            case parsing::CastKind::FloatingCast:
                breakdown.conversionRisk = 20;
                break;
            case parsing::CastKind::IntegralToBoolean:
                breakdown.conversionRisk = 15;
                break;
            case parsing::CastKind::PointerCast:
                breakdown.conversionRisk = 35;
                break;
            default:
                breakdown.conversionRisk = 10;
        }

        // Context risk - CRITICAL: This is where context-aware scoring happens
        switch (contextRecord.classification.primaryContext) {
            case context::ContextType::MEMORY_CONTEXT:
                breakdown.contextRisk = 30;  // HIGHEST
                break;
            case context::ContextType::LOOP_CONTEXT:
                breakdown.contextRisk = 25;
                break;
            case context::ContextType::API_CONTEXT:
                breakdown.contextRisk = 20;
                break;
            case context::ContextType::BRANCH_CONTEXT:
                breakdown.contextRisk = 20;
                break;
            case context::ContextType::RETURN_CONTEXT:
                breakdown.contextRisk = 15;
                break;
            case context::ContextType::ARITHMETIC_CONTEXT:
                breakdown.contextRisk = 10;
                break;
            case context::ContextType::ASSIGNMENT_CONTEXT:
                breakdown.contextRisk = 5;   // LOWEST
                break;
            default:
                breakdown.contextRisk = 5;
        }

        // Impact risk (simplified)
        breakdown.impactRisk = contextRecord.inArrayIndex ? 30 : 10;

        // Propagation risk (simplified)
        breakdown.propagationRisk = 5;

        return breakdown;
    }

    fusion::SeverityLevel computeSeverity(uint32_t score) {
        if (score >= 71) return fusion::SeverityLevel::CRITICAL;
        if (score >= 41) return fusion::SeverityLevel::HIGH;
        if (score >= 21) return fusion::SeverityLevel::MEDIUM;
        return fusion::SeverityLevel::LOW;
    }

    fusion::SuggestedFix generateFixSuggestion(const parsing::ImplicitCastEvent& event,
                                               const context::EnhancedContextRecord& contextRecord) {
        fusion::SuggestedFix fix;

        fix.description = "Use explicit static_cast<" + event.targetTypeName +
                         ">(...) to make the conversion explicit";

        fix.steps.push_back("Replace implicit conversion with explicit static_cast");

        // Add context-specific warnings
        if (contextRecord.classification.primaryContext == context::ContextType::MEMORY_CONTEXT) {
            fix.steps.push_back("CRITICAL: Validate that the converted value is within valid bounds for array indexing");
            fix.steps.push_back("Consider adding bounds checking before the conversion");
        } else if (contextRecord.classification.primaryContext == context::ContextType::LOOP_CONTEXT) {
            fix.steps.push_back("WARNING: Ensure the converted value produces correct loop bounds");
            fix.steps.push_back("Consider using the target type directly in the loop condition");
        }

        return fix;
    }

    std::string determineCategoryFromCastKind(parsing::CastKind kind) {
        switch (kind) {
            case parsing::CastKind::IntegralCast:
                return "Narrowing";
            case parsing::CastKind::FloatingCast:
                return "Narrowing";
            case parsing::CastKind::FloatingToIntegral:
                return "Narrowing";
            case parsing::CastKind::IntegralToBoolean:
                return "BooleanConversion";
            case parsing::CastKind::PointerCast:
                return "PointerConversion";
            default:
                return "Widening";
        }
    }

    clang::ASTContext* context_;
    std::vector<fusion::Finding>& findings_;
    fusion::FusionEngine* fusionEngine_;
    std::unique_ptr<context::EnhancedContextIntelligenceEngine> contextEngine_;
    bool debugContext_;
};

/**
 * Frontend Action that creates the consumer
 */
class ICHAAction : public clang::ASTFrontendAction {
public:
    explicit ICHAAction(std::vector<fusion::Finding>& findings,
                       fusion::FusionEngine* fusionEngine,
                       bool debugContext = false)
        : findings_(findings), fusionEngine_(fusionEngine), debugContext_(debugContext) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(
        clang::CompilerInstance& compiler, llvm::StringRef file) override {
        return std::make_unique<ICHAConsumer>(
            &compiler.getASTContext(), findings_, fusionEngine_, debugContext_);
    }

private:
    std::vector<fusion::Finding>& findings_;
    fusion::FusionEngine* fusionEngine_;
    bool debugContext_;
};

/**
 * Factory for creating ICHA actions
 */
class ICHAActionFactory : public clang::tooling::FrontendActionFactory {
public:
    explicit ICHAActionFactory(std::vector<fusion::Finding>& findings,
                              fusion::FusionEngine* fusionEngine,
                              bool debugContext = false)
        : findings_(findings), fusionEngine_(fusionEngine), debugContext_(debugContext) {}

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<ICHAAction>(findings_, fusionEngine_, debugContext_);
    }

private:
    std::vector<fusion::Finding>& findings_;
    fusion::FusionEngine* fusionEngine_;
    bool debugContext_;
};

} // namespace icha
