#pragma once

#include <memory>
#include <string>
#include <vector>

#include "clang/Tooling/CompilationDatabase.h"

#include "parsing/ParsingService.hpp"

namespace parsing {

class ParsingServiceImpl : public ParsingService {
   public:
    ParsingServiceImpl(std::unique_ptr<clang::tooling::CompilationDatabase> database,
                       std::vector<std::string> fallbackArgs);

    void run(const ParsingOptions& options, EventCallback callback) override;

   private:
    std::vector<std::string> resolveTranslationUnits(const ParsingOptions& options) const;

    std::unique_ptr<clang::tooling::CompilationDatabase> database_;
    std::vector<std::string> fallbackArgs_;
};

std::unique_ptr<ParsingService> makeParsingService(
    std::vector<std::string> fallbackArgs);

}  // namespace parsing
