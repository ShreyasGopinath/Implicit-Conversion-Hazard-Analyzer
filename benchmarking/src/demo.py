"""
Demo script for ICHA Benchmarking System

Demonstrates the benchmarking system with sample data
"""

import json
from pathlib import Path

from models import (
    Project, Finding, Location, SeverityLevel, ToolType,
    BenchmarkReport, CVEPattern
)
from benchmark_engine import BenchmarkEngine
from report_generator import ReportGenerator
from cve_database import get_cve_patterns


def create_sample_findings():
    """Create sample findings for demonstration"""

    # Sample ICHA findings
    icha_findings = [
        Finding(
            tool=ToolType.ICHA,
            location=Location("src/memory.cpp", 145, 10),
            source_type="int",
            target_type="unsigned int",
            category="SignChange",
            severity=SeverityLevel.CRITICAL,
            risk_score=95,
            message="Implicit conversion from 'int' to 'unsigned int' in memory access context",
            context="MEMORY_CONTEXT"
        ),
        Finding(
            tool=ToolType.ICHA,
            location=Location("src/loop.cpp", 78, 15),
            source_type="float",
            target_type="int",
            category="Narrowing",
            severity=SeverityLevel.HIGH,
            risk_score=75,
            message="Implicit conversion from 'float' to 'int' in loop condition",
            context="LOOP_CONTEXT"
        ),
        Finding(
            tool=ToolType.ICHA,
            location=Location("src/api.cpp", 203, 5),
            source_type="long",
            target_type="int",
            category="Narrowing",
            severity=SeverityLevel.HIGH,
            risk_score=80,
            message="Implicit conversion from 'long' to 'int' at API boundary",
            context="API_CONTEXT"
        ),
        Finding(
            tool=ToolType.ICHA,
            location=Location("src/calc.cpp", 56, 20),
            source_type="double",
            target_type="float",
            category="Narrowing",
            severity=SeverityLevel.MEDIUM,
            risk_score=35,
            message="Implicit conversion from 'double' to 'float'",
            context="ASSIGNMENT_CONTEXT"
        ),
        Finding(
            tool=ToolType.ICHA,
            location=Location("src/utils.cpp", 12, 8),
            source_type="int",
            target_type="long",
            category="Widening",
            severity=SeverityLevel.LOW,
            risk_score=15,
            message="Implicit widening conversion from 'int' to 'long'",
            context="ASSIGNMENT_CONTEXT"
        )
    ]

    # Sample Clang findings (some overlap with ICHA)
    clang_findings = [
        Finding(
            tool=ToolType.CLANG,
            location=Location("src/memory.cpp", 145, 10),
            source_type="int",
            target_type="unsigned int",
            category="SignChange",
            severity=SeverityLevel.HIGH,
            risk_score=0,
            message="implicit conversion changes signedness",
            context="sign-conversion"
        ),
        Finding(
            tool=ToolType.CLANG,
            location=Location("src/loop.cpp", 78, 15),
            source_type="float",
            target_type="int",
            category="Narrowing",
            severity=SeverityLevel.MEDIUM,
            risk_score=0,
            message="implicit conversion from 'float' to 'int' changes value",
            context="conversion"
        ),
        Finding(
            tool=ToolType.CLANG,
            location=Location("src/calc.cpp", 56, 20),
            source_type="double",
            target_type="float",
            category="Narrowing",
            severity=SeverityLevel.LOW,
            risk_score=0,
            message="implicit conversion loses floating-point precision",
            context="float-conversion"
        ),
        Finding(
            tool=ToolType.CLANG,
            location=Location("src/other.cpp", 99, 12),
            source_type="size_t",
            target_type="int",
            category="Narrowing",
            severity=SeverityLevel.MEDIUM,
            risk_score=0,
            message="implicit conversion loses integer precision",
            context="shorten-64-to-32"
        )
    ]

    return icha_findings, clang_findings


def run_demo():
    """Run benchmarking demo"""

    print("=" * 80)
    print("ICHA BENCHMARKING SYSTEM - DEMONSTRATION")
    print("=" * 80)
    print()

    # Create output directory
    output_dir = Path("./demo_results")
    output_dir.mkdir(exist_ok=True)

    # Create sample findings
    print("Creating sample findings...")
    icha_findings, clang_findings = create_sample_findings()

    # Create mock reports
    icha_report = BenchmarkReport(
        tool=ToolType.ICHA,
        project="demo_project",
        total_findings=len(icha_findings),
        findings=icha_findings,
        execution_time=2.5,
        files_analyzed=5
    )

    clang_report = BenchmarkReport(
        tool=ToolType.CLANG,
        project="demo_project",
        total_findings=len(clang_findings),
        findings=clang_findings,
        execution_time=1.8,
        files_analyzed=5
    )

    print(f"  ICHA:  {len(icha_findings)} findings")
    print(f"  Clang: {len(clang_findings)} findings")
    print()

    # Initialize engines
    benchmark_engine = BenchmarkEngine("./icha", str(output_dir))
    report_generator = ReportGenerator(str(output_dir))

    # Compare results
    print("Comparing ICHA vs Clang...")
    comparison = benchmark_engine.compare(icha_report, clang_report)

    print(f"  Common findings:  {comparison.metrics.common_findings}")
    print(f"  ICHA-only:        {comparison.metrics.icha_only}")
    print(f"  Clang-only:       {comparison.metrics.clang_only}")
    print(f"  Coverage:         {comparison.metrics.coverage_vs_baseline * 100:.1f}%")
    print()

    # CVE correlation
    print("Correlating with CVE patterns...")
    cve_patterns = get_cve_patterns()
    cve_report = benchmark_engine.correlate_with_cves(icha_findings, cve_patterns)

    print(f"  Total findings:       {cve_report.total_findings}")
    print(f"  Correlated:           {cve_report.correlated_findings}")
    print(f"  High confidence:      {len(cve_report.high_confidence_correlations)}")
    print(f"  Correlation rate:     {cve_report.correlation_rate * 100:.1f}%")
    print()

    # Generate reports
    print("Generating reports...")

    json_path = report_generator.generate_json_report(
        comparison, cve_report, "demo_report.json"
    )
    print(f"  ✓ JSON Report: {json_path}")

    stats_path = report_generator.generate_statistical_summary(
        [comparison], "demo_stats.txt"
    )
    print(f"  ✓ Statistical Summary: {stats_path}")

    table_path = report_generator.generate_comparison_table(
        [comparison], "demo_comparison.txt"
    )
    print(f"  ✓ Comparison Table: {table_path}")

    cve_path = report_generator.generate_cve_report(
        cve_report, "demo_cve.txt"
    )
    print(f"  ✓ CVE Report: {cve_path}")

    exec_path = report_generator.generate_executive_summary(
        [comparison], [cve_report], "demo_executive.txt"
    )
    print(f"  ✓ Executive Summary: {exec_path}")

    print()
    print("=" * 80)
    print("Demo Complete!")
    print("=" * 80)
    print()
    print(f"All reports saved to: {output_dir}")
    print()

    # Display sample from executive summary
    print("Executive Summary Preview:")
    print("-" * 80)
    with open(exec_path, 'r') as f:
        lines = f.readlines()[:30]
        print(''.join(lines))
    print("-" * 80)


if __name__ == '__main__':
    run_demo()
