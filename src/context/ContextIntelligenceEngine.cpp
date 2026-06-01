#include "context/ContextIntelligenceEngine.hpp"

#include <utility>

namespace context {
namespace {
class DefaultContextIntelligenceEngine : public ContextIntelligenceEngine {
   public:
    void analyze(const parsing::ImplicitCastEvent& event, Callback callback) const override {
        if (!callback) {
            return;
        }
        ContextRecord record{};
        record.event = event;
        record.inLoopCondition = event.metadata.inLoopCondition;
        record.inBranchCondition = event.metadata.inBranchCondition;
        record.inArrayIndex = event.metadata.inArraySubscript;
        record.inReturnStatement = event.metadata.inReturnStatement;
        record.inArithmeticExpression = event.metadata.inArithmeticExpression;
        record.inApiBoundary = event.metadata.inApiBoundary;
        record.inMemoryAllocation = event.metadata.inMemoryAllocation;
        record.inFunctionArgument = event.metadata.inFunctionArgument;
        record.isAssignmentOnly = !(record.inLoopCondition || record.inBranchCondition ||
                                    record.inArrayIndex || record.inApiBoundary ||
                                    record.inReturnStatement || record.inArithmeticExpression ||
                                    record.inFunctionArgument || record.inMemoryAllocation);
        callback(record);
    }
};
}  // namespace

std::unique_ptr<ContextIntelligenceEngine> makeContextIntelligenceEngine() {
    return std::make_unique<DefaultContextIntelligenceEngine>();
}

}  // namespace context
