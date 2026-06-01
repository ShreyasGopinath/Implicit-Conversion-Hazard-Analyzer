# Implicit Conversion Hazard Analyzer

## What is Implicit Conversion Hazard Analyzer?
The Implicit Conversion Hazard Analyzer is a highly advanced, production-grade Clang static analysis tool for C/C++ code. While standard compilers (like GCC/Clang with `-Wconversion`) indiscriminately flag all implicit conversions regardless of context (resulting in extreme noise), Implicit Conversion Hazard Analyzer uses advanced **Context Intelligence** to mathematically score the actual *danger* of a conversion. 

A narrowing conversion inside a standard assignment might be relatively harmless, but the *exact same* conversion inside an **array index** (memory context) or **loop condition** can lead to critical buffer overflows or infinite loops (e.g., CVE-2021-3156, CVE-2022-23521). Implicit Conversion Hazard Analyzer tracks the exact AST parent lineage (through declarations and statements) to isolate high-risk conversions and suggest safe fixes.

## Prerequisites
- **LLVM / Clang (LibTooling)**: `llvm@15` or equivalent.
- **CMake**: `cmake` (>= 3.14).
- **C++ Compiler**: A compiler supporting C++17/C++20.
- **Python 3**: For the interactive TUI, dashboard, and evaluation engine.
- **Python Dependencies**: Listed in `requirements-cli.txt` (`rich`, `prompt_toolkit`).

## How to Build
To build the Clang C++ analyzer binary:
```bash
./scripts/build.sh
```
This will create a `build/` directory and compile the `icha` binary inside `build/src/icha`.

## How to Run

Implicit Conversion Hazard Analyzer offers two methods of operation: an **Interactive TUI** (highly recommended) and **Direct CLI Commands**.

### Method 1: The Interactive TUI & HTML Dashboard (Recommended)
We provide an advanced, Claude-CLI inspired Terminal User Interface.
```bash
./scripts/run.sh
```
Once inside the interactive `icha❯` prompt, you have access to powerful commands:
- `/analyze <file.c>`: Runs the analysis on a custom C/C++ file and renders a beautifully formatted Risk Table directly in your terminal.
- `/html <file.c>`: Generates a premium, interactive glassmorphic HTML dashboard (`report.html`) complete with dynamic charts, search filters, collapsible AST viewers, and copyable safe-fix suggestions. It will automatically open in your default web browser.
- `/benchmark`: Triggers the automated evaluation suite to compute statistics and correlations. *(Note: The 3 large OSS codebases required for evaluation—SQLite, Redis, and Libpng—are pre-bundled in the `oss_evaluation/` directory for instant, out-of-the-box reproducibility without any external downloads).*
- `/compare`: Displays a side-by-side comparison matrix showing Implicit Conversion Hazard Analyzer's noise-reduction against the Clang compiler baseline.
- `/test`: Runs the comprehensive test suite to prove the Context Intelligence risk differentiation.

### Method 2: Direct CLI Commands (Automation/CI)
You can directly invoke the compiled Clang LibTooling binary.

**Standard Output (Terminal):**
```bash
./build/src/icha testcases/loop_context.c --
```

**JSON Output (For CI pipelines or Custom Integration):**
```bash
./build/src/icha -icha-format=json -icha-output=results.json testcases/loop_context.c --
```
*(Note: Always append `--` to the end of the command to signify the end of compiler flags for LibTooling).*
