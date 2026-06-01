#pragma once

#include <functional>
#include <memory>
#include <string>

#include "parsing/ImplicitCastEvent.hpp"
#include "parsing/ParsingOptions.hpp"

namespace parsing {

class ParsingService {
   public:
    using EventCallback = std::function<void(const ImplicitCastEvent&)>;
    virtual ~ParsingService() = default;

    virtual void run(const ParsingOptions& options, EventCallback callback) = 0;
};

std::unique_ptr<ParsingService> makeParsingService(
    std::vector<std::string> fallbackArgs = { "-std=c++20" });

}  // namespace parsing
