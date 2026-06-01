#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "impact/GraphNode.hpp"
#include "impact/ImpactPath.hpp"

namespace impact {

class ConversionImpactGraph {
   public:
    ConversionImpactGraph();
    ~ConversionImpactGraph();

    // Node management
    NodeID addConversionNode(SourceLocation location, std::string sourceType, std::string targetType);
    NodeID addVariableNode(SourceLocation location, std::string name, std::string type);
    NodeID addFunctionNode(SourceLocation location, std::string name);
    NodeID addMemoryAccessNode(SourceLocation location, MemoryAccessNode::AccessType accessType);
    NodeID addAPICallNode(SourceLocation location, std::string functionName);
    NodeID addControlFlowNode(SourceLocation location, ControlFlowNode::FlowType flowType);

    // Edge management
    void addEdge(NodeID source, NodeID target, EdgeType type);

    // Graph queries
    const GraphNode* getNode(NodeID id) const;
    std::vector<NodeID> getSuccessors(NodeID nodeId) const;
    std::vector<NodeID> getPredecessors(NodeID nodeId) const;
    std::vector<GraphEdge> getOutgoingEdges(NodeID nodeId) const;
    std::vector<GraphEdge> getIncomingEdges(NodeID nodeId) const;

    // Conversion-specific queries
    std::vector<NodeID> getAllConversionNodes() const;
    std::vector<NodeID> getVariablesDefinedByConversion(NodeID conversionId) const;

    // Propagation analysis
    PropagationSummary analyzeConversionImpact(NodeID conversionId) const;
    std::vector<ImpactPath> getImpactPaths(NodeID conversionId) const;

    // Graph statistics
    size_t nodeCount() const { return nodes_.size(); }
    size_t edgeCount() const { return edges_.size(); }

    // Debugging and visualization
    std::string toDot() const;
    void clear();

   private:
    // Internal data structures
    std::unordered_map<NodeID, std::unique_ptr<GraphNode>> nodes_;
    std::vector<GraphEdge> edges_;
    std::unordered_map<NodeID, std::vector<NodeID>> adjacencyList_;
    std::unordered_map<NodeID, std::vector<NodeID>> reverseAdjacencyList_;

    NodeID nextNodeId_;

    // Helper methods for propagation analysis
    void buildImpactPathsRecursive(
        NodeID currentNode,
        NodeID conversionId,
        std::vector<NodeID>& currentPath,
        std::vector<EdgeType>& currentEdges,
        std::unordered_set<NodeID>& visited,
        std::vector<ImpactPath>& results,
        const ConversionNode* conversionNode) const;

    bool isImpactNode(const GraphNode* node) const;
    ImpactCategory getImpactCategory(const GraphNode* node) const;

    void collectAffectedNodes(
        NodeID startNode,
        std::unordered_set<NodeID>& affectedVariables,
        std::unordered_set<NodeID>& affectedFunctions,
        std::unordered_set<NodeID>& impactPoints) const;
};

}  // namespace impact
