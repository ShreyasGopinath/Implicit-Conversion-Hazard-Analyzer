"""
ICHA Benchmarking System

Enterprise-grade evaluation and benchmarking framework for the
Implicit Conversion Hazard Analyzer (ICHA).
"""

__version__ = "1.0.0"
__author__ = "ICHA Development Team"

from .models import (
    Finding,
    Location,
    SeverityLevel,
    ToolType,
    Project,
    BenchmarkReport,
    ComparisonReport,
    ComparisonMetrics,
    CVEPattern,
    CVECorrelation,
    CVEReport
)

from .benchmark_engine import BenchmarkEngine
from .report_generator import ReportGenerator
from .parsers import ICHAParser, ClangParser, FindingNormalizer
from .cve_database import get_cve_patterns

__all__ = [
    # Models
    'Finding',
    'Location',
    'SeverityLevel',
    'ToolType',
    'Project',
    'BenchmarkReport',
    'ComparisonReport',
    'ComparisonMetrics',
    'CVEPattern',
    'CVECorrelation',
    'CVEReport',
    # Engines
    'BenchmarkEngine',
    'ReportGenerator',
    # Parsers
    'ICHAParser',
    'ClangParser',
    'FindingNormalizer',
    # Utilities
    'get_cve_patterns',
]
