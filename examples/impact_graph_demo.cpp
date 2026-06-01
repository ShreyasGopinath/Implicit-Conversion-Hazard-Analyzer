/**
 * Conversion Impact Graph Demonstration
 *
 * This example demonstrates the core innovation of ICHA:
 * tracking how values from implicit conversions propagate through programs
 * and identifying their downstream impact.
 */

#include <iostream>
#include <iomanip>

#include "impact/ConversionImpactGraph.hpp"

using namespace impact;

void printSeparator(char c = '=') {
    std::cout << std::string(80, c) << '\n';
}

void printImpactCategory(ImpactCategory category) {
    switch (category) {
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
}

void printEdgeType(EdgeType type) {
    switch (type) {
        case EdgeType::DEFINES:
            std::cout << "DEFINES";
            break;
        case EdgeType::USES:
            std::cout << "USES";
            break;
        case EdgeType::FLOWS_TO:
            std::cout << "FLOWS_TO";
            break;
        case EdgeType::CONTROLS:
            std::cout << "CONTROLS";
            break;
    }
}

void printPropagationSummary(const ConversionImpactGraph& graph,
                             const PropagationSummary& summary,
                             const std::string& scenario) {
    printSeparator();
    std::cout << "SCENARIO: " << scenario << '\n';
    printSeparator();

    const auto* convNode = dynamic_cast<const ConversionNode*>(graph.getNode(summary.conversionNodeId));
    if (convNode) {
        std::cout << "\nConversion: " << convNode->sourceType()
                  << " -> " << convNode->targetType() << '\n';
        std::cout << "Location: " << convNode->location().filePath
                  << ":" << convNode->location().line << '\n';
    }

    std::cout << "\nImpact Summary:\n";
    std::cout << "  Reaches Memory Access:  " << (summary.reachesMemoryAccess ? "YES" : "NO") << '\n';
    std::cout << "  Reaches Control Flow:   " << (summary.reachesControlFlow ? "YES" : "NO") << '\n';
    std::cout << "  Reaches API Boundary:   " << (summary.reachesAPIBoundary ? "YES" : "NO") << '\n';

    std::cout << "\nAffected Nodes:\n";
    std::cout << "  Variables: " << summary.affectedVariables.size() << '\n';
    std::cout << "  Functions: " << summary.affectedFunctions.size() << '\n';
    std::cout << "  Impact Points: " << summary.impactPoints.size() << '\n';

    std::cout << "\nPropagation Paths: " << summary.paths.size() << '\n';

    for (size_t i = 0; i < summary.paths.size(); ++i) {
        const auto& path = summary.paths[i];
        std::cout << "\n  Path " << (i + 1) << ":\n";
        std::cout << "    Length: " << path.pathNodes.size() << " nodes\n";
        std::cout << "    Final Impact: ";
        printImpactCategory(path.finalImpact.category);
        std::cout << '\n';
        std::cout << "    Impact Location: " << path.finalImpact.location.filePath
                  << ":" << path.finalImpact.location.line << '\n';
        std::cout << "    Description: " << path.finalImpact.description << '\n';

        std::cout << "\n    Propagation Chain:\n";
        for (size_t j = 0; j < path.pathNodes.size(); ++j) {
            const auto* node = graph.getNode(path.pathNodes[j]);
            if (node) {
                std::cout << "      [" << j << "] " << node->description() << '\n';
                if (j < path.pathEdges.size()) {
                    std::cout << "           ↓ ";
                    printEdgeType(path.pathEdges[j]);
                    std::cout << '\n';
                }
            }
        }
    }

    std::cout << '\n';
}

int main() {
    std::cout << "\n";
    printSeparator();
    std::cout << "  CONVERSION IMPACT GRAPH - DEMONSTRATION\n";
    printSeparator();
    std::cout << "\n";

    // ========================================================================
    // Scenario 1: Signed to Unsigned in Array Indexing
    // ========================================================================
    {
        ConversionImpactGraph graph;

        // Code: int n = -1; unsigned idx = n; arr[idx];
        auto conv = graph.addConversionNode(
            SourceLocation{"example1.cpp", 10, 1}, "int", "unsigned int");

        auto varIdx = graph.addVariableNode(
            SourceLocation{"example1.cpp", 10, 1}, "idx", "unsigned int");

        auto memAccess = graph.addMemoryAccessNode(
            SourceLocation{"example1.cpp", 11, 1},
            MemoryAccessNode::AccessType::ArrayIndex);

        graph.addEdge(conv, varIdx, EdgeType::DEFINES);
        graph.addEdge(varIdx, memAccess, EdgeType::USES);

        auto summary = graph.analyzeConversionImpact(conv);
        printPropagationSummary(graph, summary,
            "Signed to Unsigned in Array Indexing (CRITICAL)");
    }

    // ========================================================================
    // Scenario 2: Float to Int in Loop Condition
    // ========================================================================
    {
        ConversionImpactGraph graph;

        // Code: float f = 10.9; int limit = f; for(i=0; i<limit; i++)
        auto conv = graph.addConversionNode(
            SourceLocation{"example2.cpp", 20, 1}, "float", "int");

        auto varLimit = graph.addVariableNode(
            SourceLocation{"example2.cpp", 20, 1}, "limit", "int");

        auto loopCond = graph.addControlFlowNode(
            SourceLocation{"example2.cpp", 21, 1},
            ControlFlowNode::FlowType::LoopCondition);

        graph.addEdge(conv, varLimit, EdgeType::DEFINES);
        graph.addEdge(varLimit, loopCond, EdgeType::CONTROLS);

        auto summary = graph.analyzeConversionImpact(conv);
        printPropagationSummary(graph, summary,
            "Float to Int in Loop Condition (HIGH)");
    }

    // ========================================================================
    // Scenario 3: Pointer to Integer at API Boundary
    // ========================================================================
    {
        ConversionImpactGraph graph;

        // Code: void* ptr = ...; long addr = (long)ptr; syscall(addr);
        auto conv = graph.addConversionNode(
            SourceLocation{"example3.cpp", 30, 1}, "void *", "long");

        auto varAddr = graph.addVariableNode(
            SourceLocation{"example3.cpp", 30, 1}, "addr", "long");

        auto apiCall = graph.addAPICallNode(
            SourceLocation{"example3.cpp", 31, 1}, "syscall");

        graph.addEdge(conv, varAddr, EdgeType::DEFINES);
        graph.addEdge(varAddr, apiCall, EdgeType::FLOWS_TO);

        auto summary = graph.analyzeConversionImpact(conv);
        printPropagationSummary(graph, summary,
            "Pointer to Integer at API Boundary (CRITICAL)");
    }

    // ========================================================================
    // Scenario 4: Multiple Propagation Paths
    // ========================================================================
    {
        ConversionImpactGraph graph;

        // Code: double d = 3.14; int x = d; arr[x]; if(x > 0) {...}
        auto conv = graph.addConversionNode(
            SourceLocation{"example4.cpp", 40, 1}, "double", "int");

        auto varX = graph.addVariableNode(
            SourceLocation{"example4.cpp", 40, 1}, "x", "int");

        auto memAccess = graph.addMemoryAccessNode(
            SourceLocation{"example4.cpp", 41, 1},
            MemoryAccessNode::AccessType::ArrayIndex);

        auto branchCond = graph.addControlFlowNode(
            SourceLocation{"example4.cpp", 42, 1},
            ControlFlowNode::FlowType::BranchCondition);

        graph.addEdge(conv, varX, EdgeType::DEFINES);
        graph.addEdge(varX, memAccess, EdgeType::USES);
        graph.addEdge(varX, branchCond, EdgeType::CONTROLS);

        auto summary = graph.analyzeConversionImpact(conv);
        printPropagationSummary(graph, summary,
            "Multiple Propagation Paths (CRITICAL)");
    }

    // ========================================================================
    // Scenario 5: Chained Propagation Through Variables
    // ========================================================================
    {
        ConversionImpactGraph graph;

        // Code: long l = 1000000; int x = l; int y = x + 1; int z = y * 2; arr[z];
        auto conv = graph.addConversionNode(
            SourceLocation{"example5.cpp", 50, 1}, "long", "int");

        auto varX = graph.addVariableNode(
            SourceLocation{"example5.cpp", 50, 1}, "x", "int");

        auto varY = graph.addVariableNode(
            SourceLocation{"example5.cpp", 51, 1}, "y", "int");

        auto varZ = graph.addVariableNode(
            SourceLocation{"example5.cpp", 52, 1}, "z", "int");

        auto memAccess = graph.addMemoryAccessNode(
            SourceLocation{"example5.cpp", 53, 1},
            MemoryAccessNode::AccessType::ArrayIndex);

        graph.addEdge(conv, varX, EdgeType::DEFINES);
        graph.addEdge(varX, varY, EdgeType::FLOWS_TO);
        graph.addEdge(varY, varZ, EdgeType::FLOWS_TO);
        graph.addEdge(varZ, memAccess, EdgeType::USES);

        auto summary = graph.analyzeConversionImpact(conv);
        printPropagationSummary(graph, summary,
            "Chained Propagation Through Variables (HIGH)");
    }

    // ========================================================================
    // Scenario 6: Cross-Function Propagation
    // ========================================================================
    {
        ConversionImpactGraph graph;

        // Code: float f = 3.5; int x = f; processValue(x); [inside: *ptr = x]
        auto conv = graph.addConversionNode(
            SourceLocation{"example6.cpp", 60, 1}, "float", "int");

        auto varX = graph.addVariableNode(
            SourceLocation{"example6.cpp", 60, 1}, "x", "int");

        auto func = graph.addFunctionNode(
            SourceLocation{"example6.cpp", 61, 1}, "processValue");

        auto memAccess = graph.addMemoryAccessNode(
            SourceLocation{"example6.cpp", 70, 1},
            MemoryAccessNode::AccessType::PointerDereference);

        graph.addEdge(conv, varX, EdgeType::DEFINES);
        graph.addEdge(varX, func, EdgeType::FLOWS_TO);
        graph.addEdge(func, memAccess, EdgeType::FLOWS_TO);

        auto summary = graph.analyzeConversionImpact(conv);
        printPropagationSummary(graph, summary,
            "Cross-Function Propagation (HIGH)");
    }

    printSeparator();
    std::cout << "Demonstration completed successfully.\n";
    std::cout << "\nGraph Visualization:\n";
    std::cout << "  Use the toDot() method to generate GraphViz DOT format\n";
    std::cout << "  for visual representation of propagation paths.\n";
    printSeparator();
    std::cout << "\n";

    return 0;
}
