#pragma once

#include <functional>

#include "clang/AST/RecursiveASTVisitor.h"

#include "parsing/ImplicitCastEvent.hpp"

namespace parsing::detail {

class ImplicitCastVisitor : public clang::RecursiveASTVisitor<ImplicitCastVisitor> {
   public:
    explicit ImplicitCastVisitor(std::function<void(const ImplicitCastEvent&)> callback,
                                 clang::ASTContext& context);

    bool VisitImplicitCastExpr(clang::ImplicitCastExpr* expr);

   private:
    ASTMetadata captureMetadata(const clang::ImplicitCastExpr* expr) const;
    ImplicitCastEvent buildEvent(const clang::ImplicitCastExpr* expr) const;

    std::function<void(const ImplicitCastEvent&)> callback_;
    clang::ASTContext& context_;
};

}  // namespace parsing::detail
