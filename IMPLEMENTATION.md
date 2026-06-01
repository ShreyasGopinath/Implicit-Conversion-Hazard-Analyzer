# Implementation Details (LLVM/Clang)

> [!TIP]
> **For Evaluators & Reviewers:** 
> - If you are looking for **Installation & Setup Steps**, please refer to [`README.md`](file:///Users/shreyasgopinath/Desktop/CD Lab EL/README.md).
> - If you are looking for **Empirical Benchmarking Data**, please refer to [`EVALUATION.md`](file:///Users/shreyasgopinath/Desktop/CD Lab EL/EVALUATION.md).
> - If you are looking for **High-Level Design & Risk Architecture**, please refer to [`DESIGN.md`](file:///Users/shreyasgopinath/Desktop/CD Lab EL/DESIGN.md).


The Implicit Conversion Hazard Analyzer backend is a robust C++ analyzer built heavily on LLVM and Clang's **LibTooling** architecture. 

## 1. The Clang AST Visitor Interface
The core engine relies on inheriting from Clang's `RecursiveASTVisitor<T>`. 
Specifically, the system targets `VisitImplicitCastExpr(clang::ImplicitCastExpr *expr)`.
Every time the Clang AST identifies an implicit cast during syntax tree generation, Implicit Conversion Hazard Analyzer intercepts the node for analysis.

## 2. Polymorphic AST Traversal (`DynTypedNode`)
A massive hurdle in Clang AST traversal is that `Stmt` (Statements) and `Decl` (Declarations) are fundamentally different branches of the tree inheritance hierarchy. 
A naive implementation calling `expr->getParent()` fails when an implicit cast occurs inside a variable initialization (e.g., `short offset = 10;`), because the parent is a `VarDecl`, not a `Stmt`.

**Solution**: Implicit Conversion Hazard Analyzer uses `clang::DynTypedNode::create(*expr)`. This allows the engine to wrap any AST node type and traverse polymorphically upwards using `ASTContext::getParents(node)`. It walks the tree until it hits the bounding `FunctionDecl`, cataloging the complete parent lineage array.

## 3. Context Intelligence Engine
With the parent lineage captured, the `EnhancedContextIntelligenceEngine` executes pattern-matching on the node types:
- **`MEMORY_CONTEXT`**: Identified if the lineage contains an `ArraySubscriptExpr` or a pointer arithmetic `UnaryOperator`.
- **`LOOP_CONTEXT`**: Identified if the lineage contains a `ForStmt`, `WhileStmt`, or `CXXForRangeStmt`.
- **`BRANCH_CONTEXT`**: Identified by `IfStmt`, `SwitchStmt`, or `ConditionalOperator`.
- **`API_CONTEXT`**: Identified if the lineage contains a `CallExpr` where the cast affects an argument.
- **`RETURN_CONTEXT`**: Identified via `ReturnStmt`.

## 4. Diagnostics and Fix-Its
Implicit Conversion Hazard Analyzer maps the computed semantic context to safe refactoring templates.
For a conversion occurring in an array index (`MEMORY_CONTEXT`), Implicit Conversion Hazard Analyzer computes the exact file location (`SourceLocation`) via the `SourceManager` and emits a JSON payload recommending an explicit bounds-checked `static_cast<size_t>(...)`.

## 5. Execution Pipeline
1. `main()` parses command-line arguments using `llvm::cl`.
2. A `CommonOptionsParser` initializes the `ClangTool`.
3. The `ClangTool::run()` method is invoked with a custom `FrontendActionFactory`.
4. The factory spawns our `ICHAAction`, which creates an `ASTConsumer`.
5. The `ASTConsumer` utilizes the `ImplicitCastVisitor` to traverse the AST.
6. The `ReportingEngine` aggregates the findings, computes risk scores, and serializes the result via `llvm::json`.
