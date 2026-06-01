#include "parsing/ParsingServiceImpl.hpp"

#include <filesystem>
#include <stdexcept>
#include <utility>

#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/JSONCompilationDatabase.h"
#include "clang/Tooling/Tooling.h"

#include "parsing/detail/ImplicitCastVisitor.hpp"

namespace parsing {
namespace fs = std::filesystem;

namespace {
class ImplicitCastConsumer : public clang::ASTConsumer {
   public:
    explicit ImplicitCastConsumer(ParsingService::EventCallback callback)
        : callback_(std::move(callback)) {}

    void HandleTranslationUnit(clang::ASTContext& context) override {
        detail::ImplicitCastVisitor visitor(callback_, context);
        visitor.TraverseDecl(context.getTranslationUnitDecl());
    }

   private:
    ParsingService::EventCallback callback_;
};

class ImplicitCastFrontendAction : public clang::ASTFrontendAction {
   public:
    explicit ImplicitCastFrontendAction(ParsingService::EventCallback callback)
        : callback_(std::move(callback)) {}

    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& compiler,
                                                          llvm::StringRef) override {
        compiler.getDiagnostics().setIgnoreAllWarnings(true);
        return std::make_unique<ImplicitCastConsumer>(callback_);
    }

   private:
    ParsingService::EventCallback callback_;
};

class ImplicitCastActionFactory : public clang::tooling::FrontendActionFactory {
   public:
    explicit ImplicitCastActionFactory(ParsingService::EventCallback callback)
        : callback_(std::move(callback)) {}

    std::unique_ptr<clang::FrontendAction> create() override {
        return std::make_unique<ImplicitCastFrontendAction>(callback_);
    }

   private:
    ParsingService::EventCallback callback_;
};

std::unique_ptr<clang::tooling::CompilationDatabase> createCompilationDatabase(
    const ParsingOptions& options, const std::vector<std::string>& fallbackArgs) {
    std::string errorMessage;

    if (options.compilationDatabaseDirectory) {
        auto db = clang::tooling::JSONCompilationDatabase::loadFromDirectory(
            *options.compilationDatabaseDirectory, errorMessage);
        if (!db) {
            throw std::runtime_error("Failed to load JSON compilation database from " +
                                     *options.compilationDatabaseDirectory + ": " + errorMessage);
        }
        return db;
    }

    if (options.databaseStrategy == CompilationDatabaseStrategy::Fixed) {
        return std::make_unique<clang::tooling::FixedCompilationDatabase>(".", fallbackArgs);
    }

    auto db = clang::tooling::JSONCompilationDatabase::loadFromFile(
        "compile_commands.json", errorMessage, clang::tooling::JSONCommandLineSyntax::AutoDetect);
    if (!db) {
        return std::make_unique<clang::tooling::FixedCompilationDatabase>(".", fallbackArgs);
    }
    return db;
}

}  // namespace

ParsingServiceImpl::ParsingServiceImpl(std::unique_ptr<clang::tooling::CompilationDatabase> database,
                                       std::vector<std::string> fallbackArgs)
    : database_(std::move(database)), fallbackArgs_(std::move(fallbackArgs)) {}

void ParsingServiceImpl::run(const ParsingOptions& options, EventCallback callback) {
    if (!callback) {
        throw std::invalid_argument("ParsingService requires a valid callback");
    }

    if (!database_) {
        database_ = createCompilationDatabase(options, fallbackArgs_);
    }

    auto files = resolveTranslationUnits(options);
    if (files.empty()) {
        throw std::runtime_error("No translation units provided or discovered.");
    }

    clang::tooling::ClangTool tool(*database_, files);
    ImplicitCastActionFactory actionFactory(callback);
    const auto result = tool.run(&actionFactory);
    if (result != 0) {
        throw std::runtime_error("ClangTool execution failed with code " + std::to_string(result));
    }
}

std::vector<std::string> ParsingServiceImpl::resolveTranslationUnits(
    const ParsingOptions& options) const {
    if (!options.translationUnits.empty()) {
        return options.translationUnits;
    }

    if (!database_) {
        return {};
    }

    std::vector<std::string> files;
    for (const auto& command : database_->getAllCompileCommands()) {
        files.push_back(command.Filename);
    }
    return files;
}

std::unique_ptr<ParsingService> makeParsingService(std::vector<std::string> fallbackArgs) {
    ParsingOptions defaults{};
    auto database = createCompilationDatabase(defaults, fallbackArgs);
    return std::make_unique<ParsingServiceImpl>(std::move(database), std::move(fallbackArgs));
}

}  // namespace parsing
