"""
Main runner for ICHA Benchmarking System
"""

import argparse
import sys
from pathlib import Path
from typing import List

from models import Project
from benchmark_engine import BenchmarkEngine
from report_generator import ReportGenerator
from cve_database import get_cve_patterns


def load_projects_from_config(config_file: str) -> List[Project]:
    """Load project configurations from file"""
    import json

    with open(config_file, 'r') as f:
        config = json.load(f)

    projects = []
    for proj_data in config.get('projects', []):
        project = Project(
            name=proj_data['name'],
            path=proj_data['path'],
            compile_commands=proj_data.get('compile_commands'),
            source_files=proj_data.get('source_files', []),
            exclude_patterns=proj_data.get('exclude_patterns', []),
            description=proj_data.get('description', '')
        )
        projects.append(project)

    return projects


def run_benchmark(projects: List[Project], icha_binary: str, output_dir: str):
    """
    Run complete benchmark suite

    Args:
        projects: List of projects to analyze
        icha_binary: Path to ICHA binary
        output_dir: Output directory for results
    """
    print("=" * 80)
    print("ICHA BENCHMARKING SYSTEM")
    print("=" * 80)
    print()

    # Initialize engines
    benchmark_engine = BenchmarkEngine(icha_binary, output_dir)
    report_generator = ReportGenerator(output_dir)

    # Load CVE patterns
    cve_patterns = get_cve_patterns()
    print(f"Loaded {len(cve_patterns)} CVE patterns for correlation\n")

    # Run benchmarks on all projects
    comparisons = []
    cve_reports = []

    for project in projects:
        print(f"\n{'=' * 80}")
        print(f"Analyzing: {project.name}")
        print(f"{'=' * 80}\n")

        # Run ICHA analysis
        icha_report = benchmark_engine.run_icha_analysis(project)

        # Run Clang baseline
        clang_report = benchmark_engine.run_clang_baseline(project)

        # Compare results
        comparison = benchmark_engine.compare(icha_report, clang_report)
        comparisons.append(comparison)

        # CVE correlation
        cve_report = benchmark_engine.correlate_with_cves(
            icha_report.findings,
            cve_patterns
        )
        cve_reports.append(cve_report)

        # Generate per-project reports
        report_generator.generate_json_report(
            comparison,
            cve_report,
            filename=f"{project.name}_report.json"
        )

    # Generate aggregate reports
    print(f"\n{'=' * 80}")
    print("Generating Aggregate Reports")
    print(f"{'=' * 80}\n")

    stats_path = report_generator.generate_statistical_summary(comparisons)
    print(f"✓ Statistical Summary: {stats_path}")

    table_path = report_generator.generate_comparison_table(comparisons)
    print(f"✓ Comparison Table: {table_path}")

    exec_path = report_generator.generate_executive_summary(comparisons, cve_reports)
    print(f"✓ Executive Summary: {exec_path}")

    # Generate CVE reports
    for i, cve_report in enumerate(cve_reports):
        cve_path = report_generator.generate_cve_report(
            cve_report,
            filename=f"{comparisons[i].project}_cve_correlation.txt"
        )
        print(f"✓ CVE Report ({comparisons[i].project}): {cve_path}")

    print(f"\n{'=' * 80}")
    print("Benchmarking Complete!")
    print(f"{'=' * 80}\n")

    # Print summary
    total_icha = sum(c.metrics.icha_total for c in comparisons)
    total_clang = sum(c.metrics.clang_total for c in comparisons)
    total_common = sum(c.metrics.common_findings for c in comparisons)

    print("SUMMARY:")
    print(f"  Projects Analyzed:     {len(projects)}")
    print(f"  Total ICHA Findings:   {total_icha}")
    print(f"  Total Clang Findings:  {total_clang}")
    print(f"  Common Findings:       {total_common}")
    if total_clang > 0:
        print(f"  Coverage:              {(total_common / total_clang) * 100:.1f}%")
    print()


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="ICHA Benchmarking and Evaluation System"
    )

    parser.add_argument(
        '--config',
        type=str,
        required=True,
        help='Path to project configuration file (JSON)'
    )

    parser.add_argument(
        '--icha-binary',
        type=str,
        default='icha',
        help='Path to ICHA analyzer binary'
    )

    parser.add_argument(
        '--output-dir',
        type=str,
        default='./benchmark_results',
        help='Output directory for benchmark results'
    )

    parser.add_argument(
        '--project',
        type=str,
        help='Run benchmark on specific project only'
    )

    args = parser.parse_args()

    # Validate inputs
    if not Path(args.config).exists():
        print(f"Error: Configuration file not found: {args.config}")
        sys.exit(1)

    # Load projects
    try:
        projects = load_projects_from_config(args.config)
    except Exception as e:
        print(f"Error loading configuration: {e}")
        sys.exit(1)

    if not projects:
        print("Error: No projects found in configuration")
        sys.exit(1)

    # Filter by specific project if requested
    if args.project:
        projects = [p for p in projects if p.name == args.project]
        if not projects:
            print(f"Error: Project '{args.project}' not found in configuration")
            sys.exit(1)

    # Run benchmark
    try:
        run_benchmark(projects, args.icha_binary, args.output_dir)
    except KeyboardInterrupt:
        print("\n\nBenchmark interrupted by user")
        sys.exit(1)
    except Exception as e:
        print(f"\n\nError during benchmarking: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == '__main__':
    main()
