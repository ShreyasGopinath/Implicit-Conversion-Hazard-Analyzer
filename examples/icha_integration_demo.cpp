/**
 * ICHA Integration Demo
 *
 * Demonstrates how the Conversion Impact Graph and Risk Intelligence Engine
 * work together to provide comprehensive analysis of implicit conversions.
 */

#include <iostream>
#include <iomanip>

#include "impact/ConversionImpactGraph.hpp"
#include "risk/RiskEngine.hpp"
#include "context/ContextRecord.hpp"
#include "domain/conversion/ConversionRecord.hpp"

using namespace impact;

void printSeparator(char c = '=') {
    std::cout << std::string(80, c) << '\n';
}

void analyzeConversion(
    const std::string& scenario,
    const std::string& sourceType,
    const std::string& targetType,
    const impact::SourceLocation& location,
    const std::function<void(ConversionImpactGraph&, NodeID)>& buildGraph) {

    printSeparator();
    std::cout << "SCENARIO: " << scenario << '\n';
    printSeparator();

    // Step 1: Build Conversion Impact Graph
    ConversionImpactGraph graph;
    auto convId = graph.addConversionNode(location, sourceType, targetType);
    buildGraph(graph, convId);

    // Step 2: Analyze Propagation
    auto propagation = graph.analyzeConversionImpact(convId);

    std::cout << "\n[STEP 1] PROPAGATION ANALYSIS (Conversion Impact Graph)\n";
    std::cout << "  Conversion: " << sourceType << " -> " << targetType << '\n';
    std::cout << "  Location: " << location.filePath << ":" << location.line << '\n';
    std::cout << "\n  Impact Detection:\n";
    std::cout << "    Reaches Memory Access:  " << (propagation.reachesMemoryAccess ? "YES" : "NO") << '\n';
    std::cout << "    Reaches Control Flow:   " << (propagation.reachesControlFlow ? "YES" : "NO") << '\n';
    std::cout << "    Reaches API Boundary:   " << (propagation.reachesAPIBoundary ? "YES" : "NO") << '\n';
    std::cout << "\n  Propagation Paths: " << propagation.paths.size() << '\n';

    // Step 3: Build Context Record
    context::ContextRecord contextRecord{};
    contextRecord.inArrayIndex = propagation.reachesMemoryAccess;
    contextRecord.inLoopCondition = propagation.reachesControlFlow;
    contextRecord.inApiBoundary = propagation.reachesAPIBoundary;

    // Step 4: Build Conversion Record
    domain::conversion::ConversionRecord conversionRecord{};
    conversionRecord.sourceType = sourceType;
    conversionRecord.targetType = targetType;
    conversionRecord.location.filePath = location.filePath;
    conversionRecord.location.line = location.line;

    // Determine signedness
    conversionRecord.sourceIsSigned = (sourceType.find("unsigned") == std::string::npos) &&
                                      (sourceType == "int" || sourceType == "long" || sourceType == "short");
    conversionRecord.targetIsSigned = (targetType.find("unsigned") == std::string::npos) &&
                                      (targetType == "int" || targetType == "long" || targetType == "short");

    // Step 5: Build Impact Profile
    risk::ImpactProfile impactProfile{};
    impactProfile.affectsMemoryAccess = propagation.reachesMemoryAccess;
    impactProfile.affectsControlFlow = propagation.reachesControlFlow;

    // Step 6: Determine Propagation Level
    risk::PropagationLevel propagationLevel = risk::PropagationLevel::ImmediateUse;
    if (propagation.reachesAPIBoundary) {
        propagationLevel = risk::PropagationLevel::ApiBoundary;
    } else if (propagation.affectedFunctions.size() > 0) {
        propagationLevel = risk::PropagationLevel::CrossFunction;
    } else if (propagation.paths.size() > 0 && propagation.paths[0].pathNodes.size() > 3) {
        propagationLevel = risk::PropagationLevel::SameExpressionChain;
    }

    // Step 7: Perform Risk Assessment
    auto riskEngine = risk::makeRiskEngine();
    auto assessment = riskEngine->assess(conversionRecord, contextRecord, impactProfile, propagationLevel);

    std::cout << "\n[STEP 2] RISK ASSESSMENT (Risk Intelligence Engine)\n";
    std::cout << "  Risk Breakdown:\n";
    std::cout << "    Conversion Risk:  " << std::setw(3) << assessment.breakdown.conversionRisk << '\n';
    std::cout << "    Context Risk:     " << std::setw(3) << assessment.breakdown.contextRisk << '\n';
    std::cout << "    Impact Risk:      " << std::setw(3) << assessment.breakdown.impactRisk << '\n';
    std::cout << "    Propagation Risk: " << std::setw(3) << assessment.breakdown.propagationRisk << '\n';
    std::cout << "    ─────────────────────\n";
    std::cout << "    TOTAL SCORE:      " << std::setw(3) << assessment.breakdown.total() << '\n';

    std::cout << "\n  Severity: ";
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

    std::cout << "\n[STEP 3] INTEGRATED ANALYSIS\n";
    std::cout << "  Explanation:\n    " << assessment.explanation << '\n';
    std::cout << "\n  Suggested Fix:\n    " << assessment.suggestedFix << '\n';

    if (propagation.paths.size() > 0) {
        std::cout << "\n  Propagation Path Details:\n";
        for (size_t i = 0; i < std::min(propagation.paths.size(), size_t(2)); ++i) {
            const auto& path = propagation.paths[i];
            std::cout << "    Path " << (i + 1) << ": ";
            std::cout << path.pathNodes.size() << " nodes -> ";
            switch (path.finalImpact.category) {
                case ImpactCategory::MEMORY_ACCESS:
                    std::cout << "MEMORY_ACCESS";
                    break;
                case ImpactCategory::CONTROL_FLOW:
                    std::cout << "CONTROL_FLOW";
                    break;
                case ImpactCategory::API_BOUNDARY:
                    std::cout << "API_BOUNDARY";
                    break;
            }
            std::cout << '\n';
        }
    }

    std::cout << '\n';
}

int main() {
    std::cout << "\n";
    printSeparator();
    std::cout << "  ICHA INTEGRATION DEMONSTRATION\n";
    std::cout << "  Conversion Impact Graph + Risk Intelligence Engine\n";
    printSeparator();
    std::cout << "\n";

    // ========================================================================
    // Scenario 1: CRITICAL - Signed to Unsigned in Array Indexing
    // ========================================================================
    analyzeConversion(
        "Signed to Unsigned in Array Indexing",
        "int", "unsigned int",
        SourceLocation{"critical.cpp", 100, 1},
        [](ConversionImpactGraph& graph, NodeID convId) {
            auto varIdx = graph.addVariableNode(
                SourceLocation{"critical.cpp", 100, 1}, "idx", "unsigned int");
            auto memAccess = graph.addMemoryAccessNode(
                SourceLocation{"critical.cpp", 101, 1},
                MemoryAccessNode::AccessType::ArrayIndex);

            graph.addEdge(convId, varIdx, EdgeType::DEFINES);
            graph.addEdge(varIdx, memAccess, EdgeType::USES);
        }
    );

    // ========================================================================
    // Scenario 2: HIGH - Float to Int in Loop Condition
    // ========================================================================
    analyzeConversion(
        "Float to Int in Loop Condition",
        "float", "int",
        SourceLocation{"high.cpp", 200, 1},
        [](ConversionImpactGraph& graph, NodeID convId) {
            auto varLimit = graph.addVariableNode(
                SourceLocation{"high.cpp", 200, 1}, "limit", "int");
            auto loopCond = graph.addControlFlowNode(
                SourceLocation{"high.cpp", 201, 1},
                ControlFlowNode::FlowType::LoopCondition);

            graph.addEdge(convId, varLimit, EdgeType::DEFINES);
            graph.addEdge(varLimit, loopCond, EdgeType::CONTROLS);
        }
    );

    // ========================================================================
    // Scenario 3: CRITICAL - Pointer to Integer at API Boundary
    // ========================================================================
    analyzeConversion(
        "Pointer to Integer at API Boundary",
        "void *", "long",
        SourceLocation{"api.cpp", 300, 1},
        [](ConversionImpactGraph& graph, NodeID convId) {
            auto varAddr = graph.addVariableNode(
                SourceLocation{"api.cpp", 300, 1}, "addr", "long");
            auto apiCall = graph.addAPICallNode(
                SourceLocation{"api.cpp", 301, 1}, "syscall");

            graph.addEdge(convId, varAddr, EdgeType::DEFINES);
            graph.addEdge(varAddr, apiCall, EdgeType::FLOWS_TO);
        }
    );

    // ========================================================================
    // Scenario 4: CRITICAL - Multiple Impact Paths
    // ========================================================================
    analyzeConversion(
        "Multiple Impact Paths (Memory + Control Flow)",
        "double", "int",
        SourceLocation{"multi.cpp", 400, 1},
        [](ConversionImpactGraph& graph, NodeID convId) {
            auto varX = graph.addVariableNode(
                SourceLocation{"multi.cpp", 400, 1}, "x", "int");
            auto memAccess = graph.addMemoryAccessNode(
                SourceLocation{"multi.cpp", 401, 1},
                MemoryAccessNode::AccessType::ArrayIndex);
            auto branchCond = graph.addControlFlowNode(
                SourceLocation{"multi.cpp", 402, 1},
                ControlFlowNode::FlowType::BranchCondition);

            graph.addEdge(convId, varX, EdgeType::DEFINES);
            graph.addEdge(varX, memAccess, EdgeType::USES);
            graph.addEdge(varX, branchCond, EdgeType::CONTROLS);
        }
    );

    // ========================================================================
    // Scenario 5: HIGH - Chained Propagation
    // ========================================================================
    analyzeConversion(
        "Chained Propagation Through Multiple Variables",
        "long", "int",
        SourceLocation{"chain.cpp", 500, 1},
        [](ConversionImpactGraph& graph, NodeID convId) {
            auto varX = graph.addVariableNode(
                SourceLocation{"chain.cpp", 500, 1}, "x", "int");
            auto varY = graph.addVariableNode(
                SourceLocation{"chain.cpp", 501, 1}, "y", "int");
            auto varZ = graph.addVariableNode(
                SourceLocation{"chain.cpp", 502, 1}, "z", "int");
            auto memAccess = graph.addMemoryAccessNode(
                SourceLocation{"chain.cpp", 503, 1},
                MemoryAccessNode::AccessType::ArrayIndex);

            graph.addEdge(convId, varX, EdgeType::DEFINES);
            graph.addEdge(varX, varY, EdgeType::FLOWS_TO);
            graph.addEdge(varY, varZ, EdgeType::FLOWS_TO);
            graph.addEdge(varZ, memAccess, EdgeType::USES);
        }
    );

    printSeparator();
    std::cout << "\nINTEGRATION SUMMARY:\n";
    std::cout << "  1. Conversion Impact Graph discovers propagation paths\n";
    std::cout << "  2. Risk Intelligence Engine scores based on impact\n";
    std::cout << "  3. Combined analysis provides actionable insights\n";
    printSeparator();
    std::cout << "\n";

    return 0;
}
