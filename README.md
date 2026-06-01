# Implicit Conversion Hazard Analyzer

## What is Implicit Conversion Hazard Analyzer?
The Implicit Conversion Hazard Analyzer is a highly advanced Clang static analysis tool for C/C++ code. While standard compilers (like GCC/Clang with `-Wconversion`) indiscriminately flag all implicit conversions regardless of context (resulting in extreme noise), Implicit Conversion Hazard Analyzer uses advanced **Context Intelligence** to mathematically score the actual *danger* of a conversion. 

A narrowing conversion inside a standard assignment might be relatively harmless, but the *exact same* conversion inside an **array index** (memory context) or **loop condition** can lead to critical buffer overflows or infinite loops (e.g., CVE-2021-3156, CVE-2022-23521). Implicit Conversion Hazard Analyzer tracks the exact AST parent lineage (through declarations and statements) to isolate high-risk conversions and suggest safe fixes.

## Prerequisites & Environment Setup

This analyzer uses the LLVM/Clang LibTooling C++ API. You must install the Clang development libraries and configure your environment.

### macOS
```bash
brew install llvm cmake python@3.11

export LLVM_DIR=$(brew --prefix llvm)/lib/cmake/llvm
export Clang_DIR=$(brew --prefix llvm)/lib/cmake/clang
export CC=$(brew --prefix llvm)/bin/clang
export CXX=$(brew --prefix llvm)/bin/clang++
```

### Ubuntu / Debian Linux
```bash
sudo apt-get update
sudo apt-get install -y llvm-15 clang-15 libclang-15-dev cmake python3 python3-venv python3-pip

export LLVM_DIR=/usr/lib/llvm-15/cmake
export Clang_DIR=/usr/lib/cmake/clang-15
export CC=clang-15
export CXX=clang++-15
```

### Windows
Use WSL (Windows Subsystem for Linux) running Ubuntu.
```powershell
wsl --install
```
Restart your computer, open the Ubuntu terminal, and follow the **Ubuntu / Debian Linux** instructions.

---

## Build Instructions

### Option A: Automated Build
```bash
chmod +x scripts/build.sh
./scripts/build.sh
```

### Option B: Manual CMake Build
```bash
cmake -S . -B build -DLLVM_DIR=$LLVM_DIR -DClang_DIR=$Clang_DIR
cmake --build build -j $(nproc) 
ctest --test-dir build --output-on-failure
```

---

## Python TUI Setup

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements-cli.txt
```

## How to Run

Implicit Conversion Hazard Analyzer offers two methods of operation: an **Interactive TUI** (highly recommended) and **Direct CLI Commands**.

### Method 1: The Interactive TUI & HTML Dashboard (Recommended)
We provide an advanced Terminal User Interface.
```bash
./scripts/run.sh
```
Once inside the interactive `icha❯` prompt, you have access to powerful commands:
- `/analyze <file.c>`: Runs the analysis on a custom C/C++ file and renders a beautifully formatted Risk Table directly in your terminal.
- `/html <file.c>`: Generates a premium, interactive glassmorphic HTML dashboard (`report.html`) complete with dynamic charts, search filters, collapsible AST viewers, and copyable safe-fix suggestions. It will automatically open in your default web browser.
- `/benchmark`: Triggers the automated evaluation suite to compute statistics and correlations. *(Note: The 3 large OSS codebases required for evaluation—SQLite, Redis, and Libpng—are pre-bundled in the `oss_evaluation/` directory for instant, out-of-the-box reproducibility without any external downloads).*
- `/compare`: Displays a side-by-side comparison matrix showing Implicit Conversion Hazard Analyzer's noise-reduction against the Clang compiler baseline.
- `/test`: Runs the comprehensive test suite to prove the Context Intelligence risk differentiation.

### Method 2: Direct CLI Commands 
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
