"""
Data models for ICHA Benchmarking System
"""

from dataclasses import dataclass, field
from typing import List, Dict, Optional
from enum import Enum


class SeverityLevel(Enum):
    """Severity levels for findings"""
    LOW = "LOW"
    MEDIUM = "MEDIUM"
    HIGH = "HIGH"
    CRITICAL = "CRITICAL"


class ToolType(Enum):
    """Type of analysis tool"""
    ICHA = "ICHA"
    CLANG = "CLANG"


@dataclass
class Location:
    """Source code location"""
    file_path: str
    line: int
    column: int = 1

    def __str__(self):
        return f"{self.file_path}:{self.line}:{self.column}"

    def __hash__(self):
        return hash((self.file_path, self.line, self.column))

    def __eq__(self, other):
        if not isinstance(other, Location):
            return False
        return (self.file_path == other.file_path and
                self.line == other.line and
                self.column == other.column)


@dataclass
class Finding:
    """Normalized finding from any tool"""
    tool: ToolType
    location: Location
    source_type: str
    target_type: str
    category: str
    severity: SeverityLevel
    risk_score: int
    message: str
    context: Optional[str] = None

    def __hash__(self):
        return hash((self.location, self.source_type, self.target_type))

    def __eq__(self, other):
        if not isinstance(other, Finding):
            return False
        return (self.location == other.location and
                self.source_type == other.source_type and
                self.target_type == other.target_type)


@dataclass
class Project:
    """Project configuration for benchmarking"""
    name: str
    path: str
    compile_commands: Optional[str] = None  # Path to compile_commands.json
    source_files: List[str] = field(default_factory=list)
    exclude_patterns: List[str] = field(default_factory=list)
    description: str = ""


@dataclass
class BenchmarkReport:
    """Results from running a single tool"""
    tool: ToolType
    project: str
    total_findings: int
    findings: List[Finding]
    execution_time: float
    files_analyzed: int

    def findings_by_severity(self) -> Dict[SeverityLevel, int]:
        """Count findings by severity"""
        counts = {level: 0 for level in SeverityLevel}
        for finding in self.findings:
            counts[finding.severity] += 1
        return counts


@dataclass
class ComparisonMetrics:
    """Comparison metrics between two tools"""
    # Basic counts
    icha_total: int
    clang_total: int

    # Overlap analysis
    common_findings: int  # Found by both
    icha_only: int  # Found only by ICHA
    clang_only: int  # Found only by Clang

    # Quality metrics
    icha_high_severity: int  # CRITICAL + HIGH
    clang_high_severity: int

    # Precision estimates (requires manual validation)
    estimated_true_positives: int = 0
    estimated_false_positives: int = 0

    @property
    def precision(self) -> float:
        """Calculate precision if validation data available"""
        total = self.estimated_true_positives + self.estimated_false_positives
        if total == 0:
            return 0.0
        return self.estimated_true_positives / total

    @property
    def false_positive_rate(self) -> float:
        """Calculate false positive rate"""
        if self.icha_total == 0:
            return 0.0
        return self.estimated_false_positives / self.icha_total

    @property
    def coverage_vs_baseline(self) -> float:
        """Coverage relative to Clang baseline"""
        if self.clang_total == 0:
            return 0.0
        return self.common_findings / self.clang_total


@dataclass
class ComparisonReport:
    """Complete comparison between ICHA and Clang"""
    project: str
    icha_report: BenchmarkReport
    clang_report: BenchmarkReport
    metrics: ComparisonMetrics
    common_findings: List[Finding]
    icha_only_findings: List[Finding]
    clang_only_findings: List[Finding]


@dataclass
class CVEPattern:
    """Known CVE pattern for correlation"""
    cve_id: str
    description: str
    conversion_pattern: str  # e.g., "int->unsigned"
    context_pattern: str  # e.g., "memory_access"
    severity: SeverityLevel


@dataclass
class CVECorrelation:
    """Correlation between finding and CVE pattern"""
    finding: Finding
    cve_pattern: CVEPattern
    confidence: float  # 0.0 to 1.0


@dataclass
class CVEReport:
    """CVE correlation analysis"""
    total_findings: int
    correlated_findings: int
    correlations: List[CVECorrelation]
    high_confidence_correlations: List[CVECorrelation]  # confidence > 0.7

    @property
    def correlation_rate(self) -> float:
        """Percentage of findings correlated with CVE patterns"""
        if self.total_findings == 0:
            return 0.0
        return self.correlated_findings / self.total_findings
