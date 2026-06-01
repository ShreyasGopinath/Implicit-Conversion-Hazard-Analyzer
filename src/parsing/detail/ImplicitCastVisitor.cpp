#include "parsing/detail/ImplicitCastVisitor.hpp"

#include <utility>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ParentMapContext.h"
#include "clang/Basic/SourceManager.h"
#include "llvm/Support/raw_ostream.h"

namespace parsing::detail {
namespace {

// Forward declaration
bool isAncestorOf(const clang::Expr* ancestor, const clang::Expr* descendant,
                  clang::ASTContext& context);

CastKind convertCastKind(clang::CastKind clangKind) {
    using CK = clang::CastKind;
    switch (clangKind) {
        case clang::CK_IntegralCast:
            return CastKind::IntegralCast;
        case clang::CK_IntegralToBoolean:
            return CastKind::IntegralToBoolean;
        case clang::CK_FloatingCast:
            return CastKind::FloatingCast;
        case clang::CK_FloatingToIntegral:
            return CastKind::FloatingToIntegral;
        case clang::CK_LValueToRValue:
            return CastKind::LValueToRValue;
        case clang::CK_ArrayToPointerDecay:
            return CastKind::ArrayToPointer;
        case clang::CK_FunctionToPointerDecay:
            return CastKind::FunctionToPointer;
        case clang::CK_BitCast:
        case clang::CK_PointerToIntegral:
        case clang::CK_IntegralToPointer:
        case clang::CK_DerivedToBase:
        case clang::CK_BaseToDerived:
        case clang::CK_UncheckedDerivedToBase:
            return CastKind::PointerCast;
        default:
            return CastKind::Unknown;
    }
}

SourceLocationInfo buildLocation(const clang::ASTContext& context, const clang::Expr* expr) {
    const auto& sm = context.getSourceManager();
    auto loc = expr->getExprLoc();
    SourceLocationInfo info{};
    if (loc.isInvalid()) {
        return info;
    }
    const auto expansion = sm.getExpansionLoc(loc);
    const auto fileId = sm.getFileID(expansion);
    if (auto fileEntryRef = sm.getFileEntryRefForID(fileId)) {
        info.filePath = fileEntryRef->getName().str();
    } else {
        info.filePath = sm.getFilename(expansion).str();
    }
    info.line = sm.getSpellingLineNumber(expansion);
    info.column = sm.getSpellingColumnNumber(expansion);
    return info;
}

std::string typeToString(const clang::QualType& type, const clang::ASTContext& context) {
    return type.getAsString(context.getPrintingPolicy());
}

std::string dumpExpr(const clang::Expr* expr, const clang::ASTContext& context) {
    std::string buffer;
    llvm::raw_string_ostream stream(buffer);
    expr->printPretty(stream, nullptr, context.getPrintingPolicy());
    stream.flush();
    return buffer;
}

ASTMetadata analyzeContexts(const clang::ImplicitCastExpr* expr, clang::ASTContext& context) {
    ASTMetadata metadata{};

    // Traverse the entire parent chain, not just immediate parent
    clang::DynTypedNode currentNode = clang::DynTypedNode::create(*expr);
    bool foundFunction = false;
    int depth = 0;
    const int MAX_DEPTH = 50; // Prevent infinite loops

    while (!foundFunction && depth < MAX_DEPTH) {
        auto parents = context.getParents(currentNode);
        if (parents.empty()) {
            break;
        }

        const auto& parent = parents[0];
        
        std::string nodeKind = parent.getNodeKind().asStringRef().str();
        metadata.parentNodeChain.push_back(nodeKind);

        // Store immediate parent kind
        if (depth == 0) {
            metadata.parentNodeKind = nodeKind;
        }

        // Check if we've reached a function declaration
        if (parent.get<clang::FunctionDecl>() || parent.get<clang::TranslationUnitDecl>()) {
            foundFunction = true;
            break;
        }

        // Check for context-specific AST nodes
        if (const auto* stmt = parent.get<clang::Stmt>()) {
            // Loop contexts
            if (llvm::isa<clang::ForStmt>(stmt) || llvm::isa<clang::WhileStmt>(stmt) ||
                llvm::isa<clang::DoStmt>(stmt)) {
                metadata.inLoopCondition = true;
            }

            // Branch contexts
            if (llvm::isa<clang::IfStmt>(stmt) || llvm::isa<clang::ConditionalOperator>(stmt) ||
                llvm::isa<clang::SwitchStmt>(stmt)) {
                metadata.inBranchCondition = true;
            }

            // Memory contexts - HIGHEST PRIORITY
            if (llvm::isa<clang::ArraySubscriptExpr>(stmt)) {
                metadata.inArraySubscript = true;
            }

            // Return contexts
            if (llvm::isa<clang::ReturnStmt>(stmt)) {
                metadata.inReturnStatement = true;
            }

            // API boundary contexts
            if (llvm::isa<clang::CallExpr>(stmt)) {
                metadata.inApiBoundary = true;

                // Check if conversion is a function argument
                if (const auto* call = llvm::dyn_cast<clang::CallExpr>(stmt)) {
                    for (const auto* arg : call->arguments()) {
                        // Check if our conversion is within this argument
                        if (isAncestorOf(arg, expr, context)) {
                            metadata.inFunctionArgument = true;
                            break;
                        }
                    }
                }
            }

            // Memory allocation contexts
            if (llvm::isa<clang::CXXNewExpr>(stmt) || llvm::isa<clang::CXXDeleteExpr>(stmt)) {
                metadata.inMemoryAllocation = true;
            }

            // Arithmetic contexts
            if (llvm::isa<clang::BinaryOperator>(stmt) || llvm::isa<clang::UnaryOperator>(stmt)) {
                metadata.inArithmeticExpression = true;
            }
        }

        // Move to next parent
        currentNode = parent;
        depth++;
    }

    return metadata;
}

// Helper function to check if ancestor contains descendant
bool isAncestorOf(const clang::Expr* ancestor, const clang::Expr* descendant,
                  clang::ASTContext& context) {
    if (ancestor == descendant) {
        return true;
    }

    clang::DynTypedNode current = clang::DynTypedNode::create(*descendant);
    int depth = 0;
    const int MAX_DEPTH = 20;

    while (depth < MAX_DEPTH) {
        auto parents = context.getParents(current);
        if (parents.empty()) {
            return false;
        }

        const auto& parent = parents[0];
        if (const auto* stmt = parent.get<clang::Stmt>()) {
            if (stmt == ancestor) {
                return true;
            }
        }
        current = parent;
        depth++;
    }

    return false;
}
}  // namespace

ImplicitCastVisitor::ImplicitCastVisitor(std::function<void(const ImplicitCastEvent&)> callback,
                                         clang::ASTContext& context)
    : callback_(std::move(callback)), context_(context) {}

bool ImplicitCastVisitor::VisitImplicitCastExpr(clang::ImplicitCastExpr* expr) {
    callback_(buildEvent(expr));
    return true;
}

ASTMetadata ImplicitCastVisitor::captureMetadata(const clang::ImplicitCastExpr* expr) const {
    return analyzeContexts(expr, context_);
}

ImplicitCastEvent ImplicitCastVisitor::buildEvent(const clang::ImplicitCastExpr* expr) const {
    ImplicitCastEvent event{};
    event.sourceTypeName = typeToString(expr->getSubExpr()->getType(), context_);
    event.targetTypeName = typeToString(expr->getType(), context_);
    event.castKind = convertCastKind(expr->getCastKind());
    event.location = buildLocation(context_, expr);
    event.metadata = captureMetadata(expr);
    event.astNodeDebug = dumpExpr(expr, context_);
    return event;
}

}  // namespace parsing::detail
