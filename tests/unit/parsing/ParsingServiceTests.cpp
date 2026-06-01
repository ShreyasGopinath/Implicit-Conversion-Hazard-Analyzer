#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

#include "parsing/ImplicitCastEvent.hpp"
#include "parsing/ParsingOptions.hpp"
#include "parsing/ParsingService.hpp"

namespace fs = std::filesystem;

namespace {
fs::path writeSampleSource(const std::string& content) {
    auto tmp = fs::temp_directory_path() / "ichan_parser_test.cpp";
    std::ofstream ofs(tmp);
    ofs << content;
    return tmp;
}
}  // namespace

TEST(ParsingServiceTests, DetectsImplicitCasts) {
    const std::string code = R"cpp(
        int foo(double value) {
            int x = value;
            bool b = x;
            return b;
        }
    )cpp";

    const auto sourcePath = writeSampleSource(code);

    parsing::ParsingOptions options{};
    options.translationUnits = { sourcePath.string() };
    options.databaseStrategy = parsing::CompilationDatabaseStrategy::Fixed;
    options.fallbackCompileCommands = { "-std=c++20" };

    auto service = parsing::makeParsingService();

    std::vector<parsing::ImplicitCastEvent> events;
    ASSERT_NO_THROW(service->run(options, [&](const parsing::ImplicitCastEvent& event) {
        events.push_back(event);
    }));

    ASSERT_GE(events.size(), 2u);
    const bool hasIntConversion = std::any_of(events.begin(), events.end(), [](const auto& evt) {
        return evt.targetTypeName == "int";
    });
    const bool hasBoolConversion = std::any_of(events.begin(), events.end(), [](const auto& evt) {
        return evt.targetTypeName == "bool";
    });

    EXPECT_TRUE(hasIntConversion);
    EXPECT_TRUE(hasBoolConversion);
}
