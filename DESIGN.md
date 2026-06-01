# System Design & Architecture

> [!TIP]
> **For Evaluators & Reviewers:** 
> - If you are looking for **Installation & Setup Steps**, please refer to [`README.md`](file:///Users/shreyasgopinath/Desktop/CD Lab EL/README.md).
> - If you are looking for **Empirical Benchmarking Data**, please refer to [`EVALUATION.md`](file:///Users/shreyasgopinath/Desktop/CD Lab EL/EVALUATION.md).
> - If you are looking for **Clang/LLVM C++ Code Specifics**, please refer to [`IMPLEMENTATION.md`](file:///Users/shreyasgopinath/Desktop/CD Lab EL/IMPLEMENTATION.md).


## Core Approach
The objective of Implicit Conversion Hazard Analyzer is to identify and mitigate implicit conversion hazards in C/C++ by computing a Context-Aware Risk Score. Rather than identifying *all* type-cast operations like standard compiler warnings, the system maps the semantic *environment* of the cast to isolate genuine security threats.

The architecture comprises three primary tiers:
1. **The C++ Parser Tier**: Built on LLVM/Clang LibTooling. Handles deep AST traversal and extracts nodes.
2. **The Intelligence Engine Tier**: Analyzes parent statement logic (Loops, Array indexing, API boundaries, etc.) and computes a mathematical risk score (0-100) mapped to four severity dimensions (Low, Medium, High, Critical).
3. **The Presentation Tier**: A Python-based interactive shell (TUI) and an HTML Web Dashboard generator that consume the standard JSON format emitted by the Intelligence Engine to visualize the data.

## The Risk Scoring Model
The scoring engine evaluates each implicit conversion using a robust Multi-Factor Assessment Model:
- **Conversion Risk (Max 35)**: Weights the raw cast. A `SignedToUnsigned` cast or narrowing float-to-int cast carries much higher weight than a widening integer promotion.
- **Context Risk (Max 30)**: The core innovation. Evaluates where the cast occurred.
  - `MEMORY_CONTEXT` (e.g. Array Subscripts): **30 points** (High risk of Out of Bounds).
  - `LOOP_CONTEXT` (e.g. Iteration Bounds): **25 points** (High risk of infinite loops).
  - `BRANCH_CONTEXT` (e.g. Conditionals): **20 points**.
  - `ASSIGNMENT_CONTEXT` (e.g. standard variables): **5 points**.
- **Impact Risk (Max 30)**: Assesses the size discrepancy (e.g. converting 64-bit to 8-bit is highly penalized).
- **Propagation Risk (Max 5)**: Data flow tracking.

## Alternatives Considered

### 1. Regex/Lexical Analysis
- **Approach**: Scanning source text for `short x = long_val;` patterns using grep/regex.
- **Why Rejected**: Extremely brittle. Unable to resolve typedefs, macros, or track variable types mathematically. Cannot identify complex C++ AST node boundaries like `ArraySubscriptExpr` vs generic `[]` syntax.

### 2. Clang Static Analyzer (Checker API)
- **Approach**: Writing a checker for the `clang --analyze` tool.
- **Why Rejected**: While powerful for symbolic execution, it is very difficult to produce customizable JSON output, integrate external TUI tooling, and strictly define an integer risk score. Using raw LibTooling `RecursiveASTVisitor` gave us maximum control over data extraction, traversal limits, and UI integration.

### 3. Modifying the Compiler Frontend (Clang `-Wconversion`)
- **Approach**: Altering the actual Clang compiler source code to modify how `-Wconversion` emits warnings.
- **Why Rejected**: Requires users to build a custom fork of Clang (huge 40GB+ footprint, hours of compile time). Implicit Conversion Hazard Analyzer operates as an independent, portable tool that uses Clang as a library (LibTooling), making it easily distributable.
