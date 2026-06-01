# Implicit Conversion Hazard Analyzer

## What is Implicit Conversion Hazard Analyzer?
The Implicit Conversion Hazard Analyzer is a highly advanced, production-grade Clang static analysis tool for C/C++ code. While standard compilers (like GCC/Clang with `-Wconversion`) indiscriminately flag all implicit conversions regardless of context (resulting in extreme noise), Implicit Conversion Hazard Analyzer uses advanced **Context Intelligence** to mathematically score the actual *danger* of a conversion. 

A narrowing conversion inside a standard assignment might be relatively harmless, but the *exact same* conversion inside an **array index** (memory context) or **loop condition** can lead to critical buffer overflows or infinite loops (e.g., CVE-2021-3156, CVE-2022-23521). Implicit Conversion Hazard Analyzer tracks the exact AST parent lineage (through declarations and statements) to isolate high-risk conversions and suggest safe fixes.

## 1. Prerequisites & Environment Setup

Because this analyzer leverages the LLVM/Clang LibTooling C++ API, you must have the Clang development libraries installed. Below are the step-by-step instructions for your specific operating system.

### macOS (Apple Silicon & Intel)
The easiest way to install LLVM on macOS is via Homebrew.
```bash
# 1. Install dependencies
brew install llvm cmake python@3.11

# 2. Set strict environment variables so CMake finds the Homebrew LLVM instead of Apple's default Clang
export LLVM_DIR=$(brew --prefix llvm)/lib/cmake/llvm
export Clang_DIR=$(brew --prefix llvm)/lib/cmake/clang
export CC=$(brew --prefix llvm)/bin/clang
export CXX=$(brew --prefix llvm)/bin/clang++
```

### Ubuntu / Debian Linux
Use the official package manager to install LLVM 15 (or newer).
```bash
# 1. Install dependencies
sudo apt-get update
sudo apt-get install -y llvm-15 clang-15 libclang-15-dev cmake python3 python3-venv python3-pip

# 2. Set environment variables
export LLVM_DIR=/usr/lib/llvm-15/cmake
export Clang_DIR=/usr/lib/cmake/clang-15
export CC=clang-15
export CXX=clang++-15
```

### Windows
For Windows users, we highly recommend using **WSL (Windows Subsystem for Linux)** running Ubuntu, as Clang LibTooling natively interacts best in a POSIX environment.
1. Open PowerShell and run: `wsl --install`
2. Restart your computer and open the Ubuntu terminal.
3. Follow the **Ubuntu / Debian Linux** instructions above.

---

## 2. Compilation & Build Instructions

Once your environment variables are configured, you can manually build the C++ backend using CMake, or use our automated script.

### Option A: The Automated Build Script
```bash
chmod +x scripts/build.sh
./scripts/build.sh
```

### Option B: Step-by-Step Manual CMake Build
If you wish to configure the project manually (which is helpful for debugging or IDE setup):
```bash
# 1. Generate CMake build files
cmake -S . -B build -DLLVM_DIR=$LLVM_DIR -DClang_DIR=$Clang_DIR

# 2. Compile the analyzer and its subcomponents
cmake --build build -j $(nproc) 

# 3. Verify the build by running the C++ test suite
ctest --test-dir build --output-on-failure
```
This will compile the `icha` binary inside the `build/src/` directory.

---

## 3. Python TUI Setup

The Interactive Terminal User Interface (TUI) and HTML Dashboard require Python 3.
```bash
# 1. Create a virtual environment
python3 -m venv .venv

# 2. Activate it
# On Mac/Linux/WSL:
source .venv/bin/activate

# 3. Install the required Python packages
pip install -r requirements-cli.txt
```

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
