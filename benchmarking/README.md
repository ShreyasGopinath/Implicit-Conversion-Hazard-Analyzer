# ICHA Benchmarking System

Enterprise-grade evaluation and benchmarking framework for the Implicit Conversion Hazard Analyzer (ICHA).

## Overview

This system evaluates ICHA's effectiveness by comparing it against existing compiler warnings (Clang) and correlating findings with known CVE patterns.

## Features

- **Comparative Analysis**: Side-by-side comparison with Clang `-Wconversion` warnings
- **Statistical Metrics**: Precision, coverage, and false positive rate estimation
- **CVE Correlation**: Map findings to known vulnerability patterns
- **Multiple Report Formats**: JSON, text summaries, comparison tables
- **Deterministic Results**: Reproducible benchmarking across runs

## Architecture

```
benchmarking/
├── src/
│   ├── models.py              # Data models
│   ├── parsers.py             # ICHA and Clang output parsers
│   ├── benchmark_engine.py    # Core benchmarking logic
│   ├── report_generator.py    # Report generation
│   ├── cve_database.py        # CVE pattern database
│   └── run_benchmark.py       # Main runner
├── config/
│   └── projects.json          # Project configurations
├── results/                   # Benchmark outputs (generated)
└── reports/                   # Generated reports (generated)
```

## Installation

```bash
cd benchmarking
pip install -r requirements.txt
```

## Usage

### Basic Usage

```bash
python src/run_benchmark.py \
    --config config/projects.json \
    --icha-binary ../build/icha \
    --output-dir ./results
```

### Run Specific Project

```bash
python src/run_benchmark.py \
    --config config/projects.json \
    --project sqlite_subset \
    --output-dir ./results
```

## Configuration

Edit `config/projects.json` to add your projects:

```json
{
  "projects": [
    {
      "name": "my_project",
      "path": "/path/to/project",
      "compile_commands": "/path/to/compile_commands.json",
      "source_files": ["src/main.cpp"],
      "exclude_patterns": ["*/test/*"]
    }
  ]
}
```

## Output Reports

The system generates:

1. **JSON Reports** (`project_report.json`)
   - Machine-readable benchmark data
   - Detailed metrics and findings

2. **Statistical Summary** (`statistical_summary.txt`)
   - Aggregate statistics across projects
   - Per-project breakdown

3. **Comparison Table** (`comparison_table.txt`)
   - Side-by-side ICHA vs Clang comparison
   - Severity distribution

4. **CVE Correlation** (`project_cve_correlation.txt`)
   - Findings matched to CVE patterns
   - Confidence scores

5. **Executive Summary** (`executive_summary.txt`)
   - High-level findings for presentation
   - Key recommendations

## Metrics

### Precision
```
Precision = True Positives / (True Positives + False Positives)
```

### Coverage
```
Coverage = Common Findings / Total Clang Findings
```

### False Positive Rate
```
FP Rate = False Positives / Total ICHA Findings
```

## CVE Correlation

The system includes patterns for known CVE vulnerabilities related to implicit conversions:

- Integer overflow in array indexing
- Sign extension vulnerabilities
- Precision loss in critical calculations
- Negative value wraparound
- Truncation in type conversions

## Extending the System

### Adding CVE Patterns

Edit `src/cve_database.py`:

```python
CVEPattern(
    cve_id="CVE-YYYY-XXXX",
    description="Description of vulnerability",
    conversion_pattern="source->target",
    context_pattern="context_type",
    severity=SeverityLevel.HIGH
)
```

### Custom Parsers

Implement custom parsers in `src/parsers.py` for other static analysis tools.

## Requirements

- Python 3.7+
- ICHA analyzer binary
- Clang compiler (for baseline comparison)
- Test projects with source code

## Limitations

- Requires manual validation for true/false positive classification
- CVE correlation is pattern-based (heuristic)
- Clang baseline requires compilable code
- Performance depends on project size

## Research Validation

This system is designed for research validation and proving ICHA's effectiveness in academic and enterprise contexts.

## License

Part of the ICHA project.
