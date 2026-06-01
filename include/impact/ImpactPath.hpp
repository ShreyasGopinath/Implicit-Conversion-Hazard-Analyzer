#pragma once

#include <vector>
#include <string>

#include "impact/GraphNode.hpp"

namespace impact {

enum class ImpactCategory {
    MEMORY_ACCESS,
    CONTROL_FLOW,
    API_BOUNDARY
};

struct ImpactPoint {
    NodeID nodeId;
    ImpactCategory category;
    SourceLocation location;
    std::string description;
};

struct ImpactPath {
    NodeID conversionNodeId;
    std::vector<NodeID> pathNodes;
    std::vector<EdgeType> pathEdges;
    ImpactPoint finalImpact;

    std::string sourceType;
    std::string targetType;
    SourceLocation conversionLocation;
};

struct PropagationSummary {
    NodeID conversionNodeId;
    std::vector<ImpactPath> paths;
    bool reachesMemoryAccess{ false };
    bool reachesControlFlow{ false };
    bool reachesAPIBoundary{ false };

    std::vector<NodeID> affectedVariables;
    std::vector<NodeID> affectedFunctions;
    std::vector<NodeID> impactPoints;
};

}  // namespace impact
