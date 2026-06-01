#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace parsing {

enum class CompilationDatabaseStrategy {
    AutoDetect,
    Fixed
};

struct ParsingOptions {
    std::vector<std::string> translationUnits;
    std::optional<std::string> compilationDatabaseDirectory;
    std::vector<std::string> fallbackCompileCommands{ "-std=c++20" };
    CompilationDatabaseStrategy databaseStrategy{ CompilationDatabaseStrategy::AutoDetect };
    std::size_t parallelism{ 1U };
};

}  // namespace parsing
