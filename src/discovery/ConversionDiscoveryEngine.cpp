#include "discovery/ConversionDiscoveryEngine.hpp"

#include "discovery/ConversionClassifier.hpp"

namespace discovery {

namespace {
class DefaultConversionDiscoveryEngine : public ConversionDiscoveryEngine {
   public:
    explicit DefaultConversionDiscoveryEngine(ConversionClassifierPtr classifier)
        : classifier_(std::move(classifier)) {}

    void process(const parsing::ImplicitCastEvent& event,
                 RecordCallback callback) const override {
        if (!callback) {
            return;
        }
        callback(classifier_->classify(event));
    }

   private:
    ConversionClassifierPtr classifier_;
};
}  // namespace

std::unique_ptr<ConversionDiscoveryEngine> makeConversionDiscoveryEngine() {
    return std::make_unique<DefaultConversionDiscoveryEngine>(makeDefaultConversionClassifier());
}

}  // namespace discovery
