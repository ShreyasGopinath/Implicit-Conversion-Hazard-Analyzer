#include <gtest/gtest.h>

#include "impact/ConversionImpactGraph.hpp"

using namespace impact;

namespace {

SourceLocation makeLoc(const std::string& file, unsigned line, unsigned col = 1) {
    return SourceLocation{ file, line, col };
}

}  // namespace

// ============================================================================
// Basic Graph Operations Tests
// ============================================================================

TEST(ConversionImpactGraphTests, CreateEmptyGraph) {
    ConversionImpactGraph graph;
    EXPECT_EQ(graph.nodeCount(), 0u);
    EXPECT_EQ(graph.edgeCount(), 0u);
}

TEST(ConversionImpactGraphTests, AddConversionNode) {
    ConversionImpactGraph graph;
    auto nodeId = graph.addConversionNode(makeLoc("test.cpp", 10), "double", "int");

    EXPECT_EQ(graph.nodeCount(), 1u);
    EXPECT_EQ(graph.edgeCount(), 0u);

    const auto* node = graph.getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->type(), NodeType::Conversion);

    const auto* convNode = dynamic_cast<const ConversionNode*>(node);
    ASSERT_NE(convNode, nullptr);
    EXPECT_EQ(convNode->sourceType(), "double");
    EXPECT_EQ(convNode->targetType(), "int");
}

TEST(ConversionImpactGraphTests, AddVariableNode) {
    ConversionImpactGraph graph;
    auto nodeId = graph.addVariableNode(makeLoc("test.cpp", 11), "x", "int");

    const auto* node = graph.getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->type(), NodeType::Variable);

    const auto* varNode = dynamic_cast<const VariableNode*>(node);
    ASSERT_NE(varNode, nullptr);
    EXPECT_EQ(varNode->name(), "x");
    EXPECT_EQ(varNode->varType(), "int");
}

TEST(ConversionImpactGraphTests, AddMemoryAccessNode) {
    ConversionImpactGraph graph;
    auto nodeId = graph.addMemoryAccessNode(makeLoc("test.cpp", 12),
                                             MemoryAccessNode::AccessType::ArrayIndex);

    const auto* node = graph.getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->type(), NodeType::MemoryAccess);

    const auto* memNode = dynamic_cast<const MemoryAccessNode*>(node);
    ASSERT_NE(memNode, nullptr);
    EXPECT_EQ(memNode->accessType(), MemoryAccessNode::AccessType::ArrayIndex);
}

TEST(ConversionImpactGraphTests, AddControlFlowNode) {
    ConversionImpactGraph graph;
    auto nodeId = graph.addControlFlowNode(makeLoc("test.cpp", 13),
                                            ControlFlowNode::FlowType::LoopCondition);

    const auto* node = graph.getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->type(), NodeType::ControlFlow);

    const auto* cfNode = dynamic_cast<const ControlFlowNode*>(node);
    ASSERT_NE(cfNode, nullptr);
    EXPECT_EQ(cfNode->flowType(), ControlFlowNode::FlowType::LoopCondition);
}

TEST(ConversionImpactGraphTests, AddAPICallNode) {
    ConversionImpactGraph graph;
    auto nodeId = graph.addAPICallNode(makeLoc("test.cpp", 14), "malloc");

    const auto* node = graph.getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->type(), NodeType::APICall);

    const auto* apiNode = dynamic_cast<const APICallNode*>(node);
    ASSERT_NE(apiNode, nullptr);
    EXPECT_EQ(apiNode->functionName(), "malloc");
}

TEST(ConversionImpactGraphTests, AddEdge) {
    ConversionImpactGraph graph;
    auto node1 = graph.addConversionNode(makeLoc("test.cpp", 10), "double", "int");
    auto node2 = graph.addVariableNode(makeLoc("test.cpp", 11), "x", "int");

    graph.addEdge(node1, node2, EdgeType::DEFINES);

    EXPECT_EQ(graph.edgeCount(), 1u);

    auto successors = graph.getSuccessors(node1);
    ASSERT_EQ(successors.size(), 1u);
    EXPECT_EQ(successors[0], node2);

    auto predecessors = graph.getPredecessors(node2);
    ASSERT_EQ(predecessors.size(), 1u);
    EXPECT_EQ(predecessors[0], node1);
}

TEST(ConversionImpactGraphTests, AddDuplicateEdge) {
    ConversionImpactGraph graph;
    auto node1 = graph.addConversionNode(makeLoc("test.cpp", 10), "double", "int");
    auto node2 = graph.addVariableNode(makeLoc("test.cpp", 11), "x", "int");

    graph.addEdge(node1, node2, EdgeType::DEFINES);
    graph.addEdge(node1, node2, EdgeType::DEFINES);  // Duplicate

    EXPECT_EQ(graph.edgeCount(), 1u);  // Should not add duplicate
}

TEST(ConversionImpactGraphTests, GetOutgoingEdges) {
    ConversionImpactGraph graph;
    auto node1 = graph.addConversionNode(makeLoc("test.cpp", 10), "double", "int");
    auto node2 = graph.addVariableNode(makeLoc("test.cpp", 11), "x", "int");
    auto node3 = graph.addVariableNode(makeLoc("test.cpp", 12), "y", "int");

    graph.addEdge(node1, node2, EdgeType::DEFINES);
    graph.addEdge(node1, node3, EdgeType::DEFINES);

    auto edges = graph.getOutgoingEdges(node1);
    EXPECT_EQ(edges.size(), 2u);
}

TEST(ConversionImpactGraphTests, GetIncomingEdges) {
    ConversionImpactGraph graph;
    auto node1 = graph.addConversionNode(makeLoc("test.cpp", 10), "double", "int");
    auto node2 = graph.addVariableNode(makeLoc("test.cpp", 11), "x", "int");
    auto node3 = graph.addVariableNode(makeLoc("test.cpp", 12), "y", "int");

    graph.addEdge(node1, node3, EdgeType::DEFINES);
    graph.addEdge(node2, node3, EdgeType::FLOWS_TO);

    auto edges = graph.getIncomingEdges(node3);
    EXPECT_EQ(edges.size(), 2u);
}

// ============================================================================
// Propagation Analysis Tests
// ============================================================================

TEST(ConversionImpactGraphTests, SimpleLinearPropagation) {
    ConversionImpactGraph graph;

    // Conversion: double -> int
    auto conv = graph.addConversionNode(makeLoc("test.cpp", 10), "double", "int");

    // Variable: x = (int)value
    auto varX = graph.addVariableNode(makeLoc("test.cpp", 10), "x", "int");

    // Memory access: arr[x]
    auto memAccess = graph.addMemoryAccessNode(makeLoc("test.cpp", 11),
                                                MemoryAccessNode::AccessType::ArrayIndex);

    // Build graph: Conversion -> Variable -> MemoryAccess
    graph.addEdge(conv, varX, EdgeType::DEFINES);
    graph.addEdge(varX, memAccess, EdgeType::USES);

    // Analyze impact
    auto summary = graph.analyzeConversionImpact(conv);

    EXPECT_TRUE(summary.reachesMemoryAccess);
    EXPECT_FALSE(summary.reachesControlFlow);
    EXPECT_FALSE(summary.reachesAPIBoundary);

    EXPECT_EQ(summary.paths.size(), 1u);
    EXPECT_EQ(summary.paths[0].finalImpact.category, ImpactCategory::MEMORY_ACCESS);
}

TEST(ConversionImpactGraphTests, ControlFlowPropagation) {
    ConversionImpactGraph graph;

    // Conversion: float -> int
    auto conv = graph.addConversionNode(makeLoc("test.cpp", 20), "float", "int");

    // Variable: i = (int)f
    auto varI = graph.addVariableNode(makeLoc("test.cpp", 20), "i", "int");

    // Control flow: for loop condition
    auto loopCond = graph.addControlFlowNode(makeLoc("test.cpp", 21),
                                              ControlFlowNode::FlowType::LoopCondition);

    // Build graph
    graph.addEdge(conv, varI, EdgeType::DEFINES);
    graph.addEdge(varI, loopCond, EdgeType::CONTROLS);

    // Analyze impact
    auto summary = graph.analyzeConversionImpact(conv);

    EXPECT_FALSE(summary.reachesMemoryAccess);
    EXPECT_TRUE(summary.reachesControlFlow);
    EXPECT_FALSE(summary.reachesAPIBoundary);

    EXPECT_EQ(summary.paths.size(), 1u);
    EXPECT_EQ(summary.paths[0].finalImpact.category, ImpactCategory::CONTROL_FLOW);
}

TEST(ConversionImpactGraphTests, APIBoundaryPropagation) {
    ConversionImpactGraph graph;

    // Conversion: int -> unsigned int
    auto conv = graph.addConversionNode(makeLoc("test.cpp", 30), "int", "unsigned int");

    // Variable: size = (unsigned)n
    auto varSize = graph.addVariableNode(makeLoc("test.cpp", 30), "size", "unsigned int");

    // API call: malloc(size)
    auto apiCall = graph.addAPICallNode(makeLoc("test.cpp", 31), "malloc");

    // Build graph
    graph.addEdge(conv, varSize, EdgeType::DEFINES);
    graph.addEdge(varSize, apiCall, EdgeType::FLOWS_TO);

    // Analyze impact
    auto summary = graph.analyzeConversionImpact(conv);

    EXPECT_FALSE(summary.reachesMemoryAccess);
    EXPECT_FALSE(summary.reachesControlFlow);
    EXPECT_TRUE(summary.reachesAPIBoundary);

    EXPECT_EQ(summary.paths.size(), 1u);
    EXPECT_EQ(summary.paths[0].finalImpact.category, ImpactCategory::API_BOUNDARY);
}

TEST(ConversionImpactGraphTests, MultiplePropagationPaths) {
    ConversionImpactGraph graph;

    // Conversion: double -> int
    auto conv = graph.addConversionNode(makeLoc("test.cpp", 40), "double", "int");

    // Variable: x = (int)value
    auto varX = graph.addVariableNode(makeLoc("test.cpp", 40), "x", "int");

    // Path 1: Memory access
    auto memAccess = graph.addMemoryAccessNode(makeLoc("test.cpp", 41),
                                                MemoryAccessNode::AccessType::ArrayIndex);

    // Path 2: Control flow
    auto branchCond = graph.addControlFlowNode(makeLoc("test.cpp", 42),
                                                ControlFlowNode::FlowType::BranchCondition);

    // Build graph with two paths
    graph.addEdge(conv, varX, EdgeType::DEFINES);
    graph.addEdge(varX, memAccess, EdgeType::USES);
    graph.addEdge(varX, branchCond, EdgeType::CONTROLS);

    // Analyze impact
    auto summary = graph.analyzeConversionImpact(conv);

    EXPECT_TRUE(summary.reachesMemoryAccess);
    EXPECT_TRUE(summary.reachesControlFlow);
    EXPECT_FALSE(summary.reachesAPIBoundary);

    EXPECT_EQ(summary.paths.size(), 2u);
}

TEST(ConversionImpactGraphTests, ChainedPropagation) {
    ConversionImpactGraph graph;

    // Conversion: long -> int
    auto conv = graph.addConversionNode(makeLoc("test.cpp", 50), "long", "int");

    // Variable chain: x = (int)value
    auto varX = graph.addVariableNode(makeLoc("test.cpp", 50), "x", "int");

    // y = x + 1
    auto varY = graph.addVariableNode(makeLoc("test.cpp", 51), "y", "int");

    // z = y * 2
    auto varZ = graph.addVariableNode(makeLoc("test.cpp", 52), "z", "int");

    // Final use: arr[z]
    auto memAccess = graph.addMemoryAccessNode(makeLoc("test.cpp", 53),
                                                MemoryAccessNode::AccessType::ArrayIndex);

    // Build chain
    graph.addEdge(conv, varX, EdgeType::DEFINES);
    graph.addEdge(varX, varY, EdgeType::FLOWS_TO);
    graph.addEdge(varY, varZ, EdgeType::FLOWS_TO);
    graph.addEdge(varZ, memAccess, EdgeType::USES);

    // Analyze impact
    auto summary = graph.analyzeConversionImpact(conv);

    EXPECT_TRUE(summary.reachesMemoryAccess);
    EXPECT_EQ(summary.paths.size(), 1u);

    // Verify path length
    const auto& path = summary.paths[0];
    EXPECT_EQ(path.pathNodes.size(), 5u);  // conv, varX, varY, varZ, memAccess
}

TEST(ConversionImpactGraphTests, NoImpactPropagation) {
    ConversionImpactGraph graph;

    // Conversion: double -> int
    auto conv = graph.addConversionNode(makeLoc("test.cpp", 60), "double", "int");

    // Variable: x = (int)value (but never used)
    auto varX = graph.addVariableNode(makeLoc("test.cpp", 60), "x", "int");

    // Build graph with no impact nodes
    graph.addEdge(conv, varX, EdgeType::DEFINES);

    // Analyze impact
    auto summary = graph.analyzeConversionImpact(conv);

    EXPECT_FALSE(summary.reachesMemoryAccess);
    EXPECT_FALSE(summary.reachesControlFlow);
    EXPECT_FALSE(summary.reachesAPIBoundary);

    EXPECT_EQ(summary.paths.size(), 0u);  // No paths to impact nodes
}

TEST(ConversionImpactGraphTests, FunctionBoundaryPropagation) {
    ConversionImpactGraph graph;

    // Conversion: float -> int
    auto conv = graph.addConversionNode(makeLoc("test.cpp", 70), "float", "int");

    // Variable: x = (int)f
    auto varX = graph.addVariableNode(makeLoc("test.cpp", 70), "x", "int");

    // Function: processValue(x)
    auto func = graph.addFunctionNode(makeLoc("test.cpp", 71), "processValue");

    // Inside function: memory access
    auto memAccess = graph.addMemoryAccessNode(makeLoc("test.cpp", 75),
                                                MemoryAccessNode::AccessType::PointerDereference);

    // Build graph crossing function boundary
    graph.addEdge(conv, varX, EdgeType::DEFINES);
    graph.addEdge(varX, func, EdgeType::FLOWS_TO);
    graph.addEdge(func, memAccess, EdgeType::FLOWS_TO);

    // Analyze impact
    auto summary = graph.analyzeConversionImpact(conv);

    EXPECT_TRUE(summary.reachesMemoryAccess);
    EXPECT_EQ(summary.affectedFunctions.size(), 1u);
}

// ============================================================================
// Graph Query Tests
// ============================================================================

TEST(ConversionImpactGraphTests, GetAllConversionNodes) {
    ConversionImpactGraph graph;

    auto conv1 = graph.addConversionNode(makeLoc("test.cpp", 10), "double", "int");
    auto conv2 = graph.addConversionNode(makeLoc("test.cpp", 20), "float", "int");
    graph.addVariableNode(makeLoc("test.cpp", 30), "x", "int");

    auto conversions = graph.getAllConversionNodes();
    EXPECT_EQ(conversions.size(), 2u);

    EXPECT_TRUE(std::find(conversions.begin(), conversions.end(), conv1) != conversions.end());
    EXPECT_TRUE(std::find(conversions.begin(), conversions.end(), conv2) != conversions.end());
}

TEST(ConversionImpactGraphTests, GetVariablesDefinedByConversion) {
    ConversionImpactGraph graph;

    auto conv = graph.addConversionNode(makeLoc("test.cpp", 10), "double", "int");
    auto varX = graph.addVariableNode(makeLoc("test.cpp", 10), "x", "int");
    auto varY = graph.addVariableNode(makeLoc("test.cpp", 11), "y", "int");

    graph.addEdge(conv, varX, EdgeType::DEFINES);
    graph.addEdge(conv, varY, EdgeType::DEFINES);

    auto variables = graph.getVariablesDefinedByConversion(conv);
    EXPECT_EQ(variables.size(), 2u);
}

// ============================================================================
// Utility Tests
// ============================================================================

TEST(ConversionImpactGraphTests, ClearGraph) {
    ConversionImpactGraph graph;

    graph.addConversionNode(makeLoc("test.cpp", 10), "double", "int");
    graph.addVariableNode(makeLoc("test.cpp", 11), "x", "int");

    EXPECT_GT(graph.nodeCount(), 0u);

    graph.clear();

    EXPECT_EQ(graph.nodeCount(), 0u);
    EXPECT_EQ(graph.edgeCount(), 0u);
}

TEST(ConversionImpactGraphTests, GenerateDotOutput) {
    ConversionImpactGraph graph;

    auto conv = graph.addConversionNode(makeLoc("test.cpp", 10), "double", "int");
    auto varX = graph.addVariableNode(makeLoc("test.cpp", 10), "x", "int");

    graph.addEdge(conv, varX, EdgeType::DEFINES);

    std::string dot = graph.toDot();

    EXPECT_FALSE(dot.empty());
    EXPECT_NE(dot.find("digraph"), std::string::npos);
    EXPECT_NE(dot.find("DEFINES"), std::string::npos);
}

// ============================================================================
// Complex Scenario Tests
// ============================================================================

TEST(ConversionImpactGraphTests, ComplexScenario_SignedToUnsignedArrayIndex) {
    ConversionImpactGraph graph;

    // Scenario: int n = -1; unsigned idx = n; arr[idx];
    auto conv = graph.addConversionNode(makeLoc("test.cpp", 100), "int", "unsigned int");
    auto varIdx = graph.addVariableNode(makeLoc("test.cpp", 100), "idx", "unsigned int");
    auto memAccess = graph.addMemoryAccessNode(makeLoc("test.cpp", 101),
                                                MemoryAccessNode::AccessType::ArrayIndex);

    graph.addEdge(conv, varIdx, EdgeType::DEFINES);
    graph.addEdge(varIdx, memAccess, EdgeType::USES);

    auto summary = graph.analyzeConversionImpact(conv);

    EXPECT_TRUE(summary.reachesMemoryAccess);
    EXPECT_EQ(summary.paths.size(), 1u);
    EXPECT_EQ(summary.paths[0].sourceType, "int");
    EXPECT_EQ(summary.paths[0].targetType, "unsigned int");
}

TEST(ConversionImpactGraphTests, ComplexScenario_FloatToIntLoopBound) {
    ConversionImpactGraph graph;

    // Scenario: float f = 10.9; int limit = f; for(i=0; i<limit; i++)
    auto conv = graph.addConversionNode(makeLoc("test.cpp", 110), "float", "int");
    auto varLimit = graph.addVariableNode(makeLoc("test.cpp", 110), "limit", "int");
    auto loopCond = graph.addControlFlowNode(makeLoc("test.cpp", 111),
                                              ControlFlowNode::FlowType::LoopCondition);

    graph.addEdge(conv, varLimit, EdgeType::DEFINES);
    graph.addEdge(varLimit, loopCond, EdgeType::CONTROLS);

    auto summary = graph.analyzeConversionImpact(conv);

    EXPECT_TRUE(summary.reachesControlFlow);
    EXPECT_EQ(summary.paths.size(), 1u);
}

TEST(ConversionImpactGraphTests, ComplexScenario_PointerToIntegerAPI) {
    ConversionImpactGraph graph;

    // Scenario: void* ptr = ...; long addr = (long)ptr; syscall(addr);
    auto conv = graph.addConversionNode(makeLoc("test.cpp", 120), "void *", "long");
    auto varAddr = graph.addVariableNode(makeLoc("test.cpp", 120), "addr", "long");
    auto apiCall = graph.addAPICallNode(makeLoc("test.cpp", 121), "syscall");

    graph.addEdge(conv, varAddr, EdgeType::DEFINES);
    graph.addEdge(varAddr, apiCall, EdgeType::FLOWS_TO);

    auto summary = graph.analyzeConversionImpact(conv);

    EXPECT_TRUE(summary.reachesAPIBoundary);
    EXPECT_EQ(summary.paths.size(), 1u);
}
