#include "impact/ConversionImpactGraph.hpp"

#include <algorithm>
#include <queue>
#include <sstream>

namespace impact {

ConversionImpactGraph::ConversionImpactGraph() : nextNodeId_(1) {}

ConversionImpactGraph::~ConversionImpactGraph() = default;

// ============================================================================
// Node Management
// ============================================================================

NodeID ConversionImpactGraph::addConversionNode(SourceLocation location,
                                                 std::string sourceType,
                                                 std::string targetType) {
    NodeID id = nextNodeId_++;
    nodes_[id] = std::make_unique<ConversionNode>(id, std::move(location),
                                                   std::move(sourceType), std::move(targetType));
    return id;
}

NodeID ConversionImpactGraph::addVariableNode(SourceLocation location,
                                               std::string name,
                                               std::string type) {
    NodeID id = nextNodeId_++;
    nodes_[id] = std::make_unique<VariableNode>(id, std::move(location),
                                                 std::move(name), std::move(type));
    return id;
}

NodeID ConversionImpactGraph::addFunctionNode(SourceLocation location, std::string name) {
    NodeID id = nextNodeId_++;
    nodes_[id] = std::make_unique<FunctionNode>(id, std::move(location), std::move(name));
    return id;
}

NodeID ConversionImpactGraph::addMemoryAccessNode(SourceLocation location,
                                                   MemoryAccessNode::AccessType accessType) {
    NodeID id = nextNodeId_++;
    nodes_[id] = std::make_unique<MemoryAccessNode>(id, std::move(location), accessType);
    return id;
}

NodeID ConversionImpactGraph::addAPICallNode(SourceLocation location, std::string functionName) {
    NodeID id = nextNodeId_++;
    nodes_[id] = std::make_unique<APICallNode>(id, std::move(location), std::move(functionName));
    return id;
}

NodeID ConversionImpactGraph::addControlFlowNode(SourceLocation location,
                                                  ControlFlowNode::FlowType flowType) {
    NodeID id = nextNodeId_++;
    nodes_[id] = std::make_unique<ControlFlowNode>(id, std::move(location), flowType);
    return id;
}

// ============================================================================
// Edge Management
// ============================================================================

void ConversionImpactGraph::addEdge(NodeID source, NodeID target, EdgeType type) {
    // Verify nodes exist
    if (nodes_.find(source) == nodes_.end() || nodes_.find(target) == nodes_.end()) {
        return;
    }

    GraphEdge edge{ source, target, type };

    // Check for duplicate edges
    if (std::find(edges_.begin(), edges_.end(), edge) != edges_.end()) {
        return;
    }

    edges_.push_back(edge);
    adjacencyList_[source].push_back(target);
    reverseAdjacencyList_[target].push_back(source);
}

// ============================================================================
// Graph Queries
// ============================================================================

const GraphNode* ConversionImpactGraph::getNode(NodeID id) const {
    auto it = nodes_.find(id);
    return it != nodes_.end() ? it->second.get() : nullptr;
}

std::vector<NodeID> ConversionImpactGraph::getSuccessors(NodeID nodeId) const {
    auto it = adjacencyList_.find(nodeId);
    return it != adjacencyList_.end() ? it->second : std::vector<NodeID>{};
}

std::vector<NodeID> ConversionImpactGraph::getPredecessors(NodeID nodeId) const {
    auto it = reverseAdjacencyList_.find(nodeId);
    return it != reverseAdjacencyList_.end() ? it->second : std::vector<NodeID>{};
}

std::vector<GraphEdge> ConversionImpactGraph::getOutgoingEdges(NodeID nodeId) const {
    std::vector<GraphEdge> result;
    for (const auto& edge : edges_) {
        if (edge.source == nodeId) {
            result.push_back(edge);
        }
    }
    return result;
}

std::vector<GraphEdge> ConversionImpactGraph::getIncomingEdges(NodeID nodeId) const {
    std::vector<GraphEdge> result;
    for (const auto& edge : edges_) {
        if (edge.target == nodeId) {
            result.push_back(edge);
        }
    }
    return result;
}

std::vector<NodeID> ConversionImpactGraph::getAllConversionNodes() const {
    std::vector<NodeID> result;
    for (const auto& [id, node] : nodes_) {
        if (node->type() == NodeType::Conversion) {
            result.push_back(id);
        }
    }
    return result;
}

std::vector<NodeID> ConversionImpactGraph::getVariablesDefinedByConversion(NodeID conversionId) const {
    std::vector<NodeID> result;
    for (const auto& edge : edges_) {
        if (edge.source == conversionId && edge.type == EdgeType::DEFINES) {
            const auto* node = getNode(edge.target);
            if (node && node->type() == NodeType::Variable) {
                result.push_back(edge.target);
            }
        }
    }
    return result;
}

// ============================================================================
// Propagation Analysis
// ============================================================================

bool ConversionImpactGraph::isImpactNode(const GraphNode* node) const {
    if (!node) {
        return false;
    }
    return node->type() == NodeType::MemoryAccess ||
           node->type() == NodeType::ControlFlow ||
           node->type() == NodeType::APICall;
}

ImpactCategory ConversionImpactGraph::getImpactCategory(const GraphNode* node) const {
    if (!node) {
        return ImpactCategory::MEMORY_ACCESS;  // Default, shouldn't happen
    }

    switch (node->type()) {
        case NodeType::MemoryAccess:
            return ImpactCategory::MEMORY_ACCESS;
        case NodeType::ControlFlow:
            return ImpactCategory::CONTROL_FLOW;
        case NodeType::APICall:
            return ImpactCategory::API_BOUNDARY;
        default:
            return ImpactCategory::MEMORY_ACCESS;  // Default
    }
}

void ConversionImpactGraph::buildImpactPathsRecursive(
    NodeID currentNode,
    NodeID conversionId,
    std::vector<NodeID>& currentPath,
    std::vector<EdgeType>& currentEdges,
    std::unordered_set<NodeID>& visited,
    std::vector<ImpactPath>& results,
    const ConversionNode* conversionNode) const {

    const auto* node = getNode(currentNode);
    if (!node) {
        return;
    }

    // Check if this is an impact node
    if (isImpactNode(node)) {
        ImpactPath path;
        path.conversionNodeId = conversionId;
        path.pathNodes = currentPath;
        path.pathEdges = currentEdges;
        path.sourceType = conversionNode->sourceType();
        path.targetType = conversionNode->targetType();
        path.conversionLocation = conversionNode->location();

        path.finalImpact.nodeId = currentNode;
        path.finalImpact.category = getImpactCategory(node);
        path.finalImpact.location = node->location();
        path.finalImpact.description = node->description();

        results.push_back(std::move(path));
        // Continue searching for more paths through this node
    }

    // Mark as visited for this path
    visited.insert(currentNode);

    // Explore successors
    auto successors = getSuccessors(currentNode);
    for (NodeID successor : successors) {
        // Avoid cycles in the current path
        if (visited.find(successor) != visited.end()) {
            continue;
        }

        // Find edge type
        EdgeType edgeType = EdgeType::FLOWS_TO;  // Default
        for (const auto& edge : edges_) {
            if (edge.source == currentNode && edge.target == successor) {
                edgeType = edge.type;
                break;
            }
        }

        currentPath.push_back(successor);
        currentEdges.push_back(edgeType);

        buildImpactPathsRecursive(successor, conversionId, currentPath, currentEdges,
                                  visited, results, conversionNode);

        currentPath.pop_back();
        currentEdges.pop_back();
    }

    // Unmark for other paths
    visited.erase(currentNode);
}

std::vector<ImpactPath> ConversionImpactGraph::getImpactPaths(NodeID conversionId) const {
    std::vector<ImpactPath> results;

    const auto* conversionNode = dynamic_cast<const ConversionNode*>(getNode(conversionId));
    if (!conversionNode) {
        return results;
    }

    // Start DFS from the conversion node
    std::vector<NodeID> currentPath{ conversionId };
    std::vector<EdgeType> currentEdges;
    std::unordered_set<NodeID> visited;

    buildImpactPathsRecursive(conversionId, conversionId, currentPath, currentEdges,
                              visited, results, conversionNode);

    return results;
}

void ConversionImpactGraph::collectAffectedNodes(
    NodeID startNode,
    std::unordered_set<NodeID>& affectedVariables,
    std::unordered_set<NodeID>& affectedFunctions,
    std::unordered_set<NodeID>& impactPoints) const {

    std::queue<NodeID> queue;
    std::unordered_set<NodeID> visited;

    queue.push(startNode);
    visited.insert(startNode);

    while (!queue.empty()) {
        NodeID current = queue.front();
        queue.pop();

        const auto* node = getNode(current);
        if (!node) {
            continue;
        }

        // Categorize the node
        switch (node->type()) {
            case NodeType::Variable:
                affectedVariables.insert(current);
                break;
            case NodeType::Function:
                affectedFunctions.insert(current);
                break;
            case NodeType::MemoryAccess:
            case NodeType::ControlFlow:
            case NodeType::APICall:
                impactPoints.insert(current);
                break;
            default:
                break;
        }

        // Add successors to queue
        auto successors = getSuccessors(current);
        for (NodeID successor : successors) {
            if (visited.find(successor) == visited.end()) {
                visited.insert(successor);
                queue.push(successor);
            }
        }
    }
}

PropagationSummary ConversionImpactGraph::analyzeConversionImpact(NodeID conversionId) const {
    PropagationSummary summary;
    summary.conversionNodeId = conversionId;

    // Get all impact paths
    summary.paths = getImpactPaths(conversionId);

    // Analyze what categories are reached
    for (const auto& path : summary.paths) {
        switch (path.finalImpact.category) {
            case ImpactCategory::MEMORY_ACCESS:
                summary.reachesMemoryAccess = true;
                break;
            case ImpactCategory::CONTROL_FLOW:
                summary.reachesControlFlow = true;
                break;
            case ImpactCategory::API_BOUNDARY:
                summary.reachesAPIBoundary = true;
                break;
        }
    }

    // Collect all affected nodes using BFS
    std::unordered_set<NodeID> affectedVars;
    std::unordered_set<NodeID> affectedFuncs;
    std::unordered_set<NodeID> impacts;

    collectAffectedNodes(conversionId, affectedVars, affectedFuncs, impacts);

    summary.affectedVariables.assign(affectedVars.begin(), affectedVars.end());
    summary.affectedFunctions.assign(affectedFuncs.begin(), affectedFuncs.end());
    summary.impactPoints.assign(impacts.begin(), impacts.end());

    return summary;
}

// ============================================================================
// Utility Methods
// ============================================================================

void ConversionImpactGraph::clear() {
    nodes_.clear();
    edges_.clear();
    adjacencyList_.clear();
    reverseAdjacencyList_.clear();
    nextNodeId_ = 1;
}

std::string ConversionImpactGraph::toDot() const {
    std::ostringstream oss;
    oss << "digraph ConversionImpactGraph {\n";
    oss << "  rankdir=LR;\n";
    oss << "  node [shape=box];\n\n";

    // Output nodes with styling based on type
    for (const auto& [id, node] : nodes_) {
        oss << "  n" << id << " [label=\"" << node->description() << "\"";

        switch (node->type()) {
            case NodeType::Conversion:
                oss << ", style=filled, fillcolor=lightblue";
                break;
            case NodeType::Variable:
                oss << ", style=filled, fillcolor=lightgreen";
                break;
            case NodeType::Function:
                oss << ", style=filled, fillcolor=lightyellow";
                break;
            case NodeType::MemoryAccess:
                oss << ", style=filled, fillcolor=lightcoral";
                break;
            case NodeType::APICall:
                oss << ", style=filled, fillcolor=orange";
                break;
            case NodeType::ControlFlow:
                oss << ", style=filled, fillcolor=lightpink";
                break;
        }

        oss << "];\n";
    }

    oss << "\n";

    // Output edges with labels
    for (const auto& edge : edges_) {
        oss << "  n" << edge.source << " -> n" << edge.target << " [label=\"";

        switch (edge.type) {
            case EdgeType::DEFINES:
                oss << "DEFINES";
                break;
            case EdgeType::USES:
                oss << "USES";
                break;
            case EdgeType::FLOWS_TO:
                oss << "FLOWS_TO";
                break;
            case EdgeType::CONTROLS:
                oss << "CONTROLS";
                break;
        }

        oss << "\"];\n";
    }

    oss << "}\n";
    return oss.str();
}

}  // namespace impact
