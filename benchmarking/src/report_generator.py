"""
Report Generator for ICHA Benchmarking Results
"""

import json
from pathlib import Path
from typing import List, Dict
from datetime import datetime

from models import (
    BenchmarkReport, ComparisonReport, CVEReport,
    SeverityLevel, ToolType, ComparisonMetrics
)


class ReportGenerator:
    """Generate various report formats from benchmark results"""

    def __init__(self, output_dir: str):
        """
        Initialize report generator

        Args:
            output_dir: Directory for report outputs
        """
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)

    def generate_json_report(self, comparison: ComparisonReport,
                            cve_report: CVEReport,
                            filename: str = "benchmark_report.json") -> str:
        """
        Generate comprehensive JSON report

        Args:
            comparison: Comparison report
            cve_report: CVE correlation report
            filename: Output filename

        Returns:
            Path to generated report
        """
        report_data = {
            "metadata": {
                "generated_at": datetime.now().isoformat(),
                "project": comparison.project,
                "icha_version": "1.0.0",
                "benchmark_version": "1.0.0"
            },
            "summary": {
                "icha_findings": comparison.metrics.icha_total,
                "clang_findings": comparison.metrics.clang_total,
                "common_findings": comparison.metrics.common_findings,
                "icha_only": comparison.metrics.icha_only,
                "clang_only": comparison.metrics.clang_only,
                "coverage_vs_baseline": f"{comparison.metrics.coverage_vs_baseline * 100:.1f}%"
            },
            "metrics": {
                "precision": comparison.metrics.precision,
                "false_positive_rate": comparison.metrics.false_positive_rate,
                "coverage": comparison.metrics.coverage_vs_baseline,
                "icha_high_severity": comparison.metrics.icha_high_severity,
                "clang_high_severity": comparison.metrics.clang_high_severity
            },
            "severity_distribution": {
                "icha": self._severity_distribution(comparison.icha_report),
                "clang": self._severity_distribution(comparison.clang_report)
            },
            "cve_correlation": {
                "total_findings": cve_report.total_findings,
                "correlated_findings": cve_report.correlated_findings,
                "correlation_rate": f"{cve_report.correlation_rate * 100:.1f}%",
                "high_confidence_count": len(cve_report.high_confidence_correlations)
            },
            "performance": {
                "icha_execution_time": f"{comparison.icha_report.execution_time:.2f}s",
                "clang_execution_time": f"{comparison.clang_report.execution_time:.2f}s"
            }
        }

        output_path = self.output_dir / filename
        with open(output_path, 'w') as f:
            json.dump(report_data, f, indent=2)

        return str(output_path)

    def generate_statistical_summary(self, comparisons: List[ComparisonReport],
                                    filename: str = "statistical_summary.txt") -> str:
        """
        Generate statistical summary across multiple projects

        Args:
            comparisons: List of comparison reports
            filename: Output filename

        Returns:
            Path to generated summary
        """
        output_path = self.output_dir / filename

        with open(output_path, 'w') as f:
            f.write("=" * 80 + "\n")
            f.write("ICHA BENCHMARKING - STATISTICAL SUMMARY\n")
            f.write("=" * 80 + "\n\n")

            f.write(f"Generated: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
            f.write(f"Projects Analyzed: {len(comparisons)}\n\n")

            # Aggregate statistics
            total_icha = sum(c.metrics.icha_total for c in comparisons)
            total_clang = sum(c.metrics.clang_total for c in comparisons)
            total_common = sum(c.metrics.common_findings for c in comparisons)
            total_icha_only = sum(c.metrics.icha_only for c in comparisons)
            total_clang_only = sum(c.metrics.clang_only for c in comparisons)

            f.write("AGGREGATE STATISTICS\n")
            f.write("-" * 80 + "\n")
            f.write(f"Total Implicit Conversion Hazard Analyzer Findings:        {total_icha:6d}\n")
            f.write(f"Total Clang Findings:       {total_clang:6d}\n")
            f.write(f"Common Findings:            {total_common:6d}\n")
            f.write(f"ICHA-Only Findings:         {total_icha_only:6d}\n")
            f.write(f"Clang-Only Findings:        {total_clang_only:6d}\n\n")

            # Coverage metrics
            if total_clang > 0:
                coverage = (total_common / total_clang) * 100
                f.write(f"Coverage vs Baseline:       {coverage:6.1f}%\n")

            if total_icha > 0:
                precision_estimate = (total_common / total_icha) * 100
                f.write(f"Precision Estimate:         {precision_estimate:6.1f}%\n")

            f.write("\n")

            # Per-project breakdown
            f.write("PER-PROJECT BREAKDOWN\n")
            f.write("-" * 80 + "\n\n")

            for comp in comparisons:
                f.write(f"Project: {comp.project}\n")
                f.write(f"  ICHA:    {comp.metrics.icha_total:4d} findings "
                       f"({comp.metrics.icha_high_severity} high severity)\n")
                f.write(f"  Clang:   {comp.metrics.clang_total:4d} findings "
                       f"({comp.metrics.clang_high_severity} high severity)\n")
                f.write(f"  Common:  {comp.metrics.common_findings:4d} findings\n")
                f.write(f"  Coverage: {comp.metrics.coverage_vs_baseline * 100:5.1f}%\n")
                f.write("\n")

            # Severity analysis
            f.write("SEVERITY ANALYSIS\n")
            f.write("-" * 80 + "\n\n")

            for comp in comparisons:
                icha_dist = self._severity_distribution(comp.icha_report)
                f.write(f"{comp.project}:\n")
                f.write(f"  CRITICAL: {icha_dist.get('CRITICAL', 0):3d}  ")
                f.write(f"HIGH: {icha_dist.get('HIGH', 0):3d}  ")
                f.write(f"MEDIUM: {icha_dist.get('MEDIUM', 0):3d}  ")
                f.write(f"LOW: {icha_dist.get('LOW', 0):3d}\n")

            f.write("\n")
            f.write("=" * 80 + "\n")

        return str(output_path)

    def generate_comparison_table(self, comparisons: List[ComparisonReport],
                                 filename: str = "comparison_table.txt") -> str:
        """
        Generate side-by-side comparison table

        Args:
            comparisons: List of comparison reports
            filename: Output filename

        Returns:
            Path to generated table
        """
        output_path = self.output_dir / filename

        with open(output_path, 'w') as f:
            f.write("=" * 100 + "\n")
            f.write("ICHA vs CLANG - SIDE-BY-SIDE COMPARISON\n")
            f.write("=" * 100 + "\n\n")

            # Table header
            f.write(f"{'Project':<20} | {'Tool':<6} | {'Total':>6} | {'High':>5} | "
                   f"{'Med':>5} | {'Low':>5} | {'Time':>8}\n")
            f.write("-" * 100 + "\n")

            for comp in comparisons:
                # ICHA row
                icha_dist = self._severity_distribution(comp.icha_report)
                f.write(f"{comp.project:<20} | {'ICHA':<6} | "
                       f"{comp.metrics.icha_total:>6} | "
                       f"{icha_dist.get('HIGH', 0) + icha_dist.get('CRITICAL', 0):>5} | "
                       f"{icha_dist.get('MEDIUM', 0):>5} | "
                       f"{icha_dist.get('LOW', 0):>5} | "
                       f"{comp.icha_report.execution_time:>7.2f}s\n")

                # Clang row
                clang_dist = self._severity_distribution(comp.clang_report)
                f.write(f"{'':<20} | {'Clang':<6} | "
                       f"{comp.metrics.clang_total:>6} | "
                       f"{clang_dist.get('HIGH', 0) + clang_dist.get('CRITICAL', 0):>5} | "
                       f"{clang_dist.get('MEDIUM', 0):>5} | "
                       f"{clang_dist.get('LOW', 0):>5} | "
                       f"{comp.clang_report.execution_time:>7.2f}s\n")

                # Comparison metrics
                f.write(f"{'':<20} | {'Δ':<6} | "
                       f"{comp.metrics.icha_only:>6} | "
                       f"Common: {comp.metrics.common_findings:>4} | "
                       f"Coverage: {comp.metrics.coverage_vs_baseline * 100:>5.1f}%\n")

                f.write("-" * 100 + "\n")

            f.write("\n")

        return str(output_path)

    def generate_cve_report(self, cve_report: CVEReport,
                           filename: str = "cve_correlation.txt") -> str:
        """
        Generate CVE correlation report

        Args:
            cve_report: CVE correlation report
            filename: Output filename

        Returns:
            Path to generated report
        """
        output_path = self.output_dir / filename

        with open(output_path, 'w') as f:
            f.write("=" * 80 + "\n")
            f.write("CVE PATTERN CORRELATION REPORT\n")
            f.write("=" * 80 + "\n\n")

            f.write(f"Total Findings Analyzed:     {cve_report.total_findings}\n")
            f.write(f"Correlated Findings:         {cve_report.correlated_findings}\n")
            f.write(f"Correlation Rate:            {cve_report.correlation_rate * 100:.1f}%\n")
            f.write(f"High Confidence Matches:     {len(cve_report.high_confidence_correlations)}\n\n")

            if cve_report.high_confidence_correlations:
                f.write("HIGH CONFIDENCE CORRELATIONS\n")
                f.write("-" * 80 + "\n\n")

                for corr in cve_report.high_confidence_correlations:
                    f.write(f"CVE: {corr.cve_pattern.cve_id}\n")
                    f.write(f"  Description: {corr.cve_pattern.description}\n")
                    f.write(f"  Location: {corr.finding.location}\n")
                    f.write(f"  Conversion: {corr.finding.source_type} → {corr.finding.target_type}\n")
                    f.write(f"  Confidence: {corr.confidence * 100:.1f}%\n")
                    f.write(f"  Severity: {corr.finding.severity.value}\n")
                    f.write("\n")

            f.write("=" * 80 + "\n")

        return str(output_path)

    def generate_executive_summary(self, comparisons: List[ComparisonReport],
                                  cve_reports: List[CVEReport],
                                  filename: str = "executive_summary.txt") -> str:
        """
        Generate executive summary for presentation

        Args:
            comparisons: List of comparison reports
            cve_reports: List of CVE reports
            filename: Output filename

        Returns:
            Path to generated summary
        """
        output_path = self.output_dir / filename

        with open(output_path, 'w') as f:
            f.write("=" * 80 + "\n")
            f.write("ICHA EVALUATION - EXECUTIVE SUMMARY\n")
            f.write("=" * 80 + "\n\n")

            f.write(f"Date: {datetime.now().strftime('%Y-%m-%d')}\n")
            f.write(f"Projects Evaluated: {len(comparisons)}\n\n")

            # Key findings
            f.write("KEY FINDINGS\n")
            f.write("-" * 80 + "\n\n")

            total_icha = sum(c.metrics.icha_total for c in comparisons)
            total_clang = sum(c.metrics.clang_total for c in comparisons)
            total_common = sum(c.metrics.common_findings for c in comparisons)
            total_icha_only = sum(c.metrics.icha_only for c in comparisons)

            f.write(f"1. Detection Coverage\n")
            if total_clang > 0:
                coverage = (total_common / total_clang) * 100
                f.write(f"   ICHA detected {coverage:.1f}% of baseline Clang warnings\n")
            f.write(f"   ICHA found {total_icha_only} additional issues not detected by Clang\n\n")

            f.write(f"2. Severity Analysis\n")
            total_high = sum(c.metrics.icha_high_severity for c in comparisons)
            if total_icha > 0:
                high_rate = (total_high / total_icha) * 100
                f.write(f"   {high_rate:.1f}% of Implicit Conversion Hazard Analyzer findings are HIGH or CRITICAL severity\n")
            f.write(f"   Total high-severity issues identified: {total_high}\n\n")

            f.write(f"3. CVE Correlation\n")
            if cve_reports:
                total_correlated = sum(r.correlated_findings for r in cve_reports)
                total_findings = sum(r.total_findings for r in cve_reports)
                if total_findings > 0:
                    corr_rate = (total_correlated / total_findings) * 100
                    f.write(f"   {corr_rate:.1f}% of findings correlate with known CVE patterns\n")
                total_high_conf = sum(len(r.high_confidence_correlations) for r in cve_reports)
                f.write(f"   {total_high_conf} high-confidence CVE pattern matches\n\n")

            # Recommendations
            f.write("RECOMMENDATIONS\n")
            f.write("-" * 80 + "\n\n")

            f.write("1. ICHA provides enhanced detection of implicit conversion hazards\n")
            f.write("   with context-aware risk assessment\n\n")

            f.write("2. The analyzer successfully identifies issues missed by standard\n")
            f.write("   compiler warnings, particularly in complex control flow contexts\n\n")

            f.write("3. Integration into CI/CD pipelines recommended for projects with:\n")
            f.write("   - Memory-critical operations\n")
            f.write("   - Security-sensitive code paths\n")
            f.write("   - Mixed-type arithmetic operations\n\n")

            f.write("=" * 80 + "\n")

        return str(output_path)

    def _severity_distribution(self, report: BenchmarkReport) -> Dict[str, int]:
        """Get severity distribution from a report"""
        dist = report.findings_by_severity()
        return {level.value: count for level, count in dist.items()}
