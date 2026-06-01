#pragma once

#include <cstdint>
#include <string>
#include <memory>

namespace impact {

enum class NodeType {
    Conversion,
    Variable,
    Function,
    MemoryAccess,
    APICall,
    ControlFlow
};

enum class EdgeType {
    DEFINES,
    USES,
    FLOWS_TO,
    CONTROLS
};

using NodeID = std::uint64_t;

struct SourceLocation {
    std::string filePath;
    unsigned line{ 0 };
    unsigned column{ 0 };

    bool operator==(const SourceLocation& other) const {
        return filePath == other.filePath && line == other.line && column == other.column;
    }
};

class GraphNode {
   public:
    explicit GraphNode(NodeID id, NodeType type, SourceLocation location)
        : id_(id), type_(type), location_(std::move(location)) {}

    virtual ~GraphNode() = default;

    NodeID id() const { return id_; }
    NodeType type() const { return type_; }
    const SourceLocation& location() const { return location_; }

    virtual std::string description() const = 0;

   protected:
    NodeID id_;
    NodeType type_;
    SourceLocation location_;
};

class ConversionNode : public GraphNode {
   public:
    ConversionNode(NodeID id, SourceLocation location, std::string sourceType, std::string targetType)
        : GraphNode(id, NodeType::Conversion, std::move(location)),
          sourceType_(std::move(sourceType)),
          targetType_(std::move(targetType)) {}

    const std::string& sourceType() const { return sourceType_; }
    const std::string& targetType() const { return targetType_; }

    std::string description() const override {
        return "Conversion: " + sourceType_ + " -> " + targetType_;
    }

   private:
    std::string sourceType_;
    std::string targetType_;
};

class VariableNode : public GraphNode {
   public:
    VariableNode(NodeID id, SourceLocation location, std::string name, std::string type)
        : GraphNode(id, NodeType::Variable, std::move(location)),
          name_(std::move(name)),
          type_(std::move(type)) {}

    const std::string& name() const { return name_; }
    const std::string& varType() const { return type_; }

    std::string description() const override {
        return "Variable: " + name_ + " (" + type_ + ")";
    }

   private:
    std::string name_;
    std::string type_;
};

class FunctionNode : public GraphNode {
   public:
    FunctionNode(NodeID id, SourceLocation location, std::string name)
        : GraphNode(id, NodeType::Function, std::move(location)),
          name_(std::move(name)) {}

    const std::string& name() const { return name_; }

    std::string description() const override {
        return "Function: " + name_;
    }

   private:
    std::string name_;
};

class MemoryAccessNode : public GraphNode {
   public:
    enum class AccessType { ArrayIndex, PointerDereference, MemoryAllocation };

    MemoryAccessNode(NodeID id, SourceLocation location, AccessType accessType)
        : GraphNode(id, NodeType::MemoryAccess, std::move(location)),
          accessType_(accessType) {}

    AccessType accessType() const { return accessType_; }

    std::string description() const override {
        std::string typeStr;
        switch (accessType_) {
            case AccessType::ArrayIndex:
                typeStr = "ArrayIndex";
                break;
            case AccessType::PointerDereference:
                typeStr = "PointerDereference";
                break;
            case AccessType::MemoryAllocation:
                typeStr = "MemoryAllocation";
                break;
        }
        return "MemoryAccess: " + typeStr;
    }

   private:
    AccessType accessType_;
};

class APICallNode : public GraphNode {
   public:
    APICallNode(NodeID id, SourceLocation location, std::string functionName)
        : GraphNode(id, NodeType::APICall, std::move(location)),
          functionName_(std::move(functionName)) {}

    const std::string& functionName() const { return functionName_; }

    std::string description() const override {
        return "APICall: " + functionName_;
    }

   private:
    std::string functionName_;
};

class ControlFlowNode : public GraphNode {
   public:
    enum class FlowType { LoopCondition, BranchCondition, SwitchCondition };

    ControlFlowNode(NodeID id, SourceLocation location, FlowType flowType)
        : GraphNode(id, NodeType::ControlFlow, std::move(location)),
          flowType_(flowType) {}

    FlowType flowType() const { return flowType_; }

    std::string description() const override {
        std::string typeStr;
        switch (flowType_) {
            case FlowType::LoopCondition:
                typeStr = "LoopCondition";
                break;
            case FlowType::BranchCondition:
                typeStr = "BranchCondition";
                break;
            case FlowType::SwitchCondition:
                typeStr = "SwitchCondition";
                break;
        }
        return "ControlFlow: " + typeStr;
    }

   private:
    FlowType flowType_;
};

struct GraphEdge {
    NodeID source;
    NodeID target;
    EdgeType type;

    bool operator==(const GraphEdge& other) const {
        return source == other.source && target == other.target && type == other.type;
    }
};

}  // namespace impact
