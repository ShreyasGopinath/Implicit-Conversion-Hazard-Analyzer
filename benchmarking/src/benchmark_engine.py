"""
Core Benchmarking Engine for ICHA Evaluation
"""

import subprocess
import time
import json
from pathlib import Path
from typing import List, Dict, Optional, Tuple

from models import (
    Project, Finding, BenchmarkReport, ComparisonReport,
    ComparisonMetrics, ToolType, SeverityLevel, CVEReport,
    CVEPattern, CVECorrelation
)
from parsers import ICHAParser, ClangParser, FindingNormalizer


class BenchmarkEngine:
    """Main benchmarking engine for ICHA evaluation"""

    def __init__(self, icha_binary: str, output_dir: str):
        """
        Initialize benchmark engine

        Args:
            icha_binary: Path to ICHA analyzer binary
            output_dir: Directory for benchmark outputs
        """
        self.icha_binary = icha_binary
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(parents=True, exist_ok=True)

    def run_icha_analysis(self, project: Project) -> BenchmarkReport:
        """
        Run Implicit Conversion Hazard Analyzer analysis on a project

        Args:
            project: Project configuration

        Returns:
            BenchmarkReport with Implicit Conversion Hazard Analyzer findings
        """
        print(f"Running Implicit Conversion Hazard Analyzer analysis on {project.name}...")

        start_time = time.time()

        # Prepare output paths
        json_output = self.output_dir / f"{project.name}_icha.json"
        sarif_output = self.output_dir / f"{project.name}_icha.sarif"

        # Run ICHA (this is a placeholder - actual implementation depends on ICHA CLI)
        # For now, we'll assume ICHA can be run with:
        # icha --project <path> --output-json <json> --output-sarif <sarif>

        try:
            # Run the actual ICHA binary on the project sources
            findings = self._run_icha_analysis(project, json_output)

            execution_time = time.time() - start_time

            report = BenchmarkReport(
                tool=ToolType.ICHA,
                project=project.name,
                total_findings=len(findings),
                findings=findings,
                execution_time=execution_time,
                files_analyzed=len(project.source_files) if project.source_files else 0
            )

            print(f"  ✓ Found {len(findings)} issues in {execution_time:.2f}s")
            return report

        except Exception as e:
            print(f"  ✗ Implicit Conversion Hazard Analyzer analysis failed: {e}")
            return BenchmarkReport(
                tool=ToolType.ICHA,
                project=project.name,
                total_findings=0,
                findings=[],
                execution_time=time.time() - start_time,
                files_analyzed=0
            )

    def run_clang_baseline(self, project: Project) -> BenchmarkReport:
        """
        Run Clang baseline warnings on a project

        Args:
            project: Project configuration

        Returns:
            BenchmarkReport with Clang findings
        """
        print(f"Running Clang baseline on {project.name}...")

        start_time = time.time()

        # Prepare output path
        warnings_output = self.output_dir / f"{project.name}_clang_warnings.txt"

        try:
            findings = self._run_clang_warnings(project, warnings_output)

            execution_time = time.time() - start_time

            report = BenchmarkReport(
                tool=ToolType.CLANG,
                project=project.name,
                total_findings=len(findings),
                findings=findings,
                execution_time=execution_time,
                files_analyzed=len(project.source_files) if project.source_files else 0
            )

            print(f"  ✓ Found {len(findings)} warnings in {execution_time:.2f}s")
            return report

        except Exception as e:
            print(f"  ✗ Clang analysis failed: {e}")
            return BenchmarkReport(
                tool=ToolType.CLANG,
                project=project.name,
                total_findings=0,
                findings=[],
                execution_time=time.time() - start_time,
                files_analyzed=0
            )

    def compare(self, icha_report: BenchmarkReport,
                clang_report: BenchmarkReport) -> ComparisonReport:
        """
        Compare ICHA and Clang results

        Args:
            icha_report: ICHA benchmark report
            clang_report: Clang benchmark report

        Returns:
            ComparisonReport with detailed comparison
        """
        print(f"Comparing results for {icha_report.project}...")

        # Normalize findings for comparison
        icha_findings = [
            self._normalize_finding(f, icha_report.project)
            for f in icha_report.findings
        ]
        clang_findings = [
            self._normalize_finding(f, clang_report.project)
            for f in clang_report.findings
        ]

        # Find overlaps
        common = []
        icha_only = []
        clang_only = []

        # Match Implicit Conversion Hazard Analyzer findings with Clang findings
        matched_clang = set()

        for icha_finding in icha_findings:
            found_match = False
            for i, clang_finding in enumerate(clang_findings):
                if i in matched_clang:
                    continue

                if FindingNormalizer.are_findings_equivalent(
                    icha_finding, clang_finding, location_tolerance=2
                ):
                    common.append(icha_finding)
                    matched_clang.add(i)
                    found_match = True
                    break

            if not found_match:
                icha_only.append(icha_finding)

        # Remaining Clang findings are Clang-only
        for i, clang_finding in enumerate(clang_findings):
            if i not in matched_clang:
                clang_only.append(clang_finding)

        # Count high severity findings
        icha_high = sum(
            1 for f in icha_findings
            if f.severity in [SeverityLevel.HIGH, SeverityLevel.CRITICAL]
        )
        clang_high = sum(
            1 for f in clang_findings
            if f.severity in [SeverityLevel.HIGH, SeverityLevel.CRITICAL]
        )

        metrics = ComparisonMetrics(
            icha_total=len(icha_findings),
            clang_total=len(clang_findings),
            common_findings=len(common),
            icha_only=len(icha_only),
            clang_only=len(clang_only),
            icha_high_severity=icha_high,
            clang_high_severity=clang_high
        )

        report = ComparisonReport(
            project=icha_report.project,
            icha_report=icha_report,
            clang_report=clang_report,
            metrics=metrics,
            common_findings=common,
            icha_only_findings=icha_only,
            clang_only_findings=clang_only
        )

        print(f"  ✓ Common: {len(common)}, Analyzer-only: {len(icha_only)}, Clang-only: {len(clang_only)}")

        return report

    def correlate_with_cves(self, findings: List[Finding],
                           cve_patterns: List[CVEPattern]) -> CVEReport:
        """
        Correlate findings with known CVE patterns

        Args:
            findings: List of findings to analyze
            cve_patterns: Known CVE patterns

        Returns:
            CVEReport with correlation analysis
        """
        print(f"Correlating {len(findings)} findings with {len(cve_patterns)} CVE patterns...")

        correlations = []

        for finding in findings:
            for pattern in cve_patterns:
                confidence = self._calculate_cve_confidence(finding, pattern)

                if confidence > 0.3:  # Minimum threshold
                    correlation = CVECorrelation(
                        finding=finding,
                        cve_pattern=pattern,
                        confidence=confidence
                    )
                    correlations.append(correlation)

        # Filter high confidence correlations
        high_confidence = [c for c in correlations if c.confidence > 0.7]

        # Count unique findings with correlations
        correlated_findings = len(set(c.finding for c in correlations))

        report = CVEReport(
            total_findings=len(findings),
            correlated_findings=correlated_findings,
            correlations=correlations,
            high_confidence_correlations=high_confidence
        )

        print(f"  ✓ Found {len(correlations)} correlations ({len(high_confidence)} high confidence)")

        return report

    def _get_project_sources(self, project: Project) -> List[str]:
        """Resolve all source files for a project"""
        source_files = []
        if project.source_files:
            for sf in project.source_files:
                source_files.append(sf)
        elif project.compile_commands and Path(project.compile_commands).exists():
            try:
                with open(project.compile_commands, 'r') as f:
                    db = json.load(f)
                for item in db:
                    if 'file' in item:
                        # Make path relative to project.path or keep it absolute
                        fpath = item['file']
                        try:
                            rel_path = Path(fpath).relative_to(Path(project.path).resolve())
                            source_files.append(str(rel_path))
                        except ValueError:
                            source_files.append(fpath)
            except Exception as e:
                print(f"  Warning: Failed to parse compilation db: {e}")
        
        # If still empty, scan directory recursively for C/C++ files
        if not source_files:
            import glob
            path_obj = Path(project.path)
            for ext in ('**/*.c', '**/*.cpp', '**/*.cc'):
                for f in path_obj.glob(ext):
                    if f.is_file():
                        try:
                            rel_path = f.relative_to(path_obj)
                            source_files.append(str(rel_path))
                        except ValueError:
                            source_files.append(str(f))

        # Filter out exclude patterns
        if project.exclude_patterns and source_files:
            import fnmatch
            filtered_files = []
            for sf in source_files:
                exclude = False
                for pat in project.exclude_patterns:
                    if fnmatch.fnmatch(sf, pat):
                        exclude = True
                        break
                if not exclude:
                    filtered_files.append(sf)
            source_files = filtered_files

        return source_files

    def _run_icha_analysis(self, project: Project, output_path: Path) -> List[Finding]:
        """
        Run the real ICHA binary on the project source files.
        """
        source_files = self._get_project_sources(project)
        if not source_files:
            print("  Warning: No source files found to analyze.")
            return []

        # Construct call command
        # command: icha_binary -icha-format=json -icha-output=out <source_files> --
        cmd = [self.icha_binary, "-icha-format=json", f"-icha-output={output_path}"]
        if project.compile_commands and Path(project.compile_commands).exists():
            cmd.extend(["-p", str(Path(project.compile_commands).parent)])
        
        # Add files as absolute or relative paths
        for sf in source_files:
            if Path(sf).is_absolute():
                cmd.append(sf)
            else:
                cmd.append(str((Path(project.path) / sf).resolve()))
        cmd.append("--")
        cmd.extend([
            "-isysroot", "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk",
            "-I/Library/Developer/CommandLineTools/usr/lib/clang/15.0.0/include",
            f"-I{Path(project.path).resolve()}"
        ])

        try:
            # Run the batch command
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
            if result.returncode != 0:
                print(f"  Warning: ICHA exited with code {result.returncode}")
                
            if output_path.exists():
                return ICHAParser.parse_json(str(output_path))
                
            print("  Warning: Batch analysis aborted. Falling back to sequential file analysis...")
            all_findings = []
            
            for i, sf in enumerate(source_files):
                if i % 10 == 0:
                    print(f"    Processing file {i+1}/{len(source_files)}...")
                temp_path = output_path.parent / f"temp_icha_{Path(sf).name}.json"
                seq_cmd = [self.icha_binary, "-icha-format=json", f"-icha-output={temp_path}"]
                if project.compile_commands and Path(project.compile_commands).exists():
                    seq_cmd.extend(["-p", str(Path(project.compile_commands).parent)])
                
                if Path(sf).is_absolute():
                    seq_cmd.append(sf)
                else:
                    seq_cmd.append(str((Path(project.path) / sf).resolve()))
                    
                seq_cmd.append("--")
                seq_cmd.extend([
                    "-isysroot", "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk",
                    "-I/Library/Developer/CommandLineTools/usr/lib/clang/15.0.0/include",
                    f"-I{Path(project.path).resolve()}"
                ])
                
                subprocess.run(seq_cmd, capture_output=True, text=True, timeout=60)
                
                if temp_path.exists():
                    all_findings.extend(ICHAParser.parse_json(str(temp_path)))
                    temp_path.unlink()
                    
            return all_findings

        except subprocess.TimeoutExpired:
            print("  Warning: Implicit Conversion Hazard Analyzer analysis timed out.")
        except Exception as e:
            print(f"  Warning: Failed to run ICHA: {e}")

        return []

    def _run_clang_warnings(self, project: Project,
                           output_path: Path) -> List[Finding]:
        """
        Run Clang with conversion warnings enabled
        """
        # Resolve all project sources (including from compilation database or directory scanning)
        source_files = self._get_project_sources(project)
        
        # Temporary modify project.source_files so we can reuse _run_clang_on_sources
        original_sources = project.source_files
        project.source_files = source_files
        try:
            warnings = self._run_clang_on_sources(project, output_path)
        finally:
            project.source_files = original_sources
            
        return warnings

    def _run_clang_on_sources(self, project: Project,
                              output_path: Path) -> List[Finding]:
        """Run Clang directly on source files"""
        warnings = []

        for source_file in project.source_files:
            source_path = (Path(project.path) / source_file).resolve()

            if not source_path.exists():
                continue

            # Run clang with conversion warnings
            cmd = [
                'clang',
                '-Wconversion',
                '-Wsign-compare',
                '-Wsign-conversion',
                '-Wfloat-conversion',
                '-fsyntax-only',
                "-isysroot", "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk",
                "-I/Library/Developer/CommandLineTools/usr/lib/clang/15.0.0/include",
                f"-I{Path(project.path).resolve()}",
                str(source_path)
            ]

            try:
                result = subprocess.run(
                    cmd,
                    capture_output=True,
                    text=True,
                    timeout=60
                )

                # Parse stderr for warnings
                if result.stderr:
                    # Write to temporary file for parsing (flatten name to avoid missing dirs)
                    safe_name = source_file.replace('/', '_').replace('\\', '_')
                    temp_file = output_path.parent / f"temp_{safe_name}.txt"
                    temp_file.write_text(result.stderr)
                    file_warnings = ClangParser.parse_warnings(str(temp_file))
                    warnings.extend(file_warnings)
                    temp_file.unlink()

            except subprocess.TimeoutExpired:
                print(f"  Warning: Clang timed out on {source_file}")
            except Exception as e:
                print(f"  Warning: Failed to run Clang on {source_file}: {e}")

        # Save all warnings
        if warnings:
            with open(output_path, 'w') as f:
                for w in warnings:
                    f.write(f"{w.location}: {w.message}\n")

        return warnings

    def _normalize_finding(self, finding: Finding, project_name: str) -> Finding:
        """Normalize a finding for comparison"""
        normalized_location = FindingNormalizer.normalize_location(
            finding.location
        )

        return Finding(
            tool=finding.tool,
            location=normalized_location,
            source_type=FindingNormalizer.normalize_type(finding.source_type),
            target_type=FindingNormalizer.normalize_type(finding.target_type),
            category=finding.category,
            severity=finding.severity,
            risk_score=finding.risk_score,
            message=finding.message,
            context=finding.context
        )

    def _calculate_cve_confidence(self, finding: Finding,
                                  pattern: CVEPattern) -> float:
        """
        Calculate confidence that a finding matches a CVE pattern

        Returns:
            Confidence score from 0.0 to 1.0
        """
        confidence = 0.0

        # Check conversion pattern match
        finding_conversion = f"{finding.source_type}->{finding.target_type}"
        if pattern.conversion_pattern.lower() in finding_conversion.lower():
            confidence += 0.4

        # Check context match
        if finding.context and pattern.context_pattern.lower() in finding.context.lower():
            confidence += 0.3

        # Check severity match
        if finding.severity == pattern.severity:
            confidence += 0.2

        # Check category relevance
        if pattern.conversion_pattern.lower() in finding.category.lower():
            confidence += 0.1

        return min(confidence, 1.0)
