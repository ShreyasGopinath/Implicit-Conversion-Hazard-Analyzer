"""
Parsers for ICHA and Clang output formats
"""

import json
import re
from typing import List, Optional
from pathlib import Path

from models import Finding, Location, SeverityLevel, ToolType


class ICHAParser:
    """Parse ICHA JSON/SARIF output"""

    @staticmethod
    def parse_json(json_path: str) -> List[Finding]:
        """Parse ICHA JSON report"""
        findings = []

        with open(json_path, 'r') as f:
            data = json.load(f)

        for finding_data in data.get('findings', []):
            try:
                file_path = finding_data['location']['file']
                
                # Skip system headers/SDK files to keep results clean and relevant to the project
                fpath_lower = file_path.lower()
                if any(sys_path in fpath_lower for sys_path in (
                    '/usr/include', 'commandlinetools', 'macosx.sdk', 
                    '/xcode', 'clang/15.0.0', '/usr/lib', 'include/c++'
                )):
                    continue

                location = Location(
                    file_path=file_path,
                    line=finding_data['location']['line'],
                    column=finding_data['location'].get('column', 1)
                )

                severity_str = finding_data['severity']
                severity = SeverityLevel[severity_str]

                finding = Finding(
                    tool=ToolType.ICHA,
                    location=location,
                    source_type=finding_data['conversion']['source'],
                    target_type=finding_data['conversion']['target'],
                    category=finding_data['conversion']['category'],
                    severity=severity,
                    risk_score=finding_data['risk']['finalRisk'],
                    message=finding_data.get('explanation', ''),
                    context=finding_data.get('context', '')
                )

                findings.append(finding)

            except (KeyError, ValueError) as e:
                print(f"Warning: Failed to parse finding: {e}")
                continue

        return findings

    @staticmethod
    def parse_sarif(sarif_path: str) -> List[Finding]:
        """Parse ICHA SARIF report"""
        findings = []

        with open(sarif_path, 'r') as f:
            data = json.load(f)

        for run in data.get('runs', []):
            for result in run.get('results', []):
                try:
                    # Extract location
                    location_data = result['locations'][0]['physicalLocation']
                    location = Location(
                        file_path=location_data['artifactLocation']['uri'],
                        line=location_data['region']['startLine'],
                        column=location_data['region'].get('startColumn', 1)
                    )

                    # Extract properties
                    props = result.get('properties', {})

                    # Map SARIF level to severity
                    level = result.get('level', 'warning')
                    severity_map = {
                        'note': SeverityLevel.LOW,
                        'warning': SeverityLevel.MEDIUM,
                        'error': SeverityLevel.HIGH
                    }

                    # Use actual severity from properties if available
                    if 'severity' in props:
                        severity = SeverityLevel[props['severity']]
                    else:
                        severity = severity_map.get(level, SeverityLevel.MEDIUM)

                    # Extract conversion info from message or properties
                    message = result['message']['text']

                    # Try to extract types from message
                    type_match = re.search(r"from '(.+?)' to '(.+?)'", message)
                    if type_match:
                        source_type = type_match.group(1)
                        target_type = type_match.group(2)
                    else:
                        source_type = "unknown"
                        target_type = "unknown"

                    finding = Finding(
                        tool=ToolType.ICHA,
                        location=location,
                        source_type=source_type,
                        target_type=target_type,
                        category=props.get('conversionCategory', 'Unknown'),
                        severity=severity,
                        risk_score=props.get('riskScore', 0),
                        message=message,
                        context=props.get('context', '')
                    )

                    findings.append(finding)

                except (KeyError, ValueError, IndexError) as e:
                    print(f"Warning: Failed to parse SARIF result: {e}")
                    continue

        return findings


class ClangParser:
    """Parse Clang compiler warnings"""

    # Clang warning patterns
    WARNING_PATTERN = re.compile(
        r"^(.+?):(\d+):(\d+):\s+warning:\s+(.+?)(?:\s+\[-W(.+?)\])?$"
    )

    CONVERSION_KEYWORDS = [
        'conversion',
        'implicit',
        'sign-compare',
        'sign-conversion',
        'float-conversion',
        'shorten-64-to-32'
    ]

    @staticmethod
    def parse_warnings(warning_file: str) -> List[Finding]:
        """Parse Clang warning output"""
        findings = []

        with open(warning_file, 'r') as f:
            lines = f.readlines()

        for line in lines:
            line = line.strip()
            match = ClangParser.WARNING_PATTERN.match(line)

            if not match:
                continue

            file_path = match.group(1)
            
            # Skip system headers/SDK files to keep results clean and relevant to the project
            fpath_lower = file_path.lower()
            if any(sys_path in fpath_lower for sys_path in (
                '/usr/include', 'commandlinetools', 'macosx.sdk', 
                '/xcode', 'clang/15.0.0', '/usr/lib', 'include/c++'
            )):
                continue

            line_num = int(match.group(2))
            column = int(match.group(3))
            message = match.group(4)
            warning_flag = match.group(5) if match.group(5) else ""

            # Filter for conversion-related warnings
            is_conversion = any(
                keyword in message.lower() or keyword in warning_flag.lower()
                for keyword in ClangParser.CONVERSION_KEYWORDS
            )

            if not is_conversion:
                continue

            # Extract type information from message
            source_type, target_type = ClangParser._extract_types(message)

            # Determine category
            category = ClangParser._determine_category(message, warning_flag)

            # Map to severity (Clang doesn't provide this, use heuristics)
            severity = ClangParser._estimate_severity(message, warning_flag)

            location = Location(
                file_path=file_path,
                line=line_num,
                column=column
            )

            finding = Finding(
                tool=ToolType.CLANG,
                location=location,
                source_type=source_type,
                target_type=target_type,
                category=category,
                severity=severity,
                risk_score=0,  # Clang doesn't provide risk scores
                message=message,
                context=warning_flag
            )

            findings.append(finding)

        return findings

    @staticmethod
    def _extract_types(message: str) -> tuple:
        """Extract source and target types from warning message"""
        # Pattern: "from 'type1' to 'type2'"
        match = re.search(r"from '(.+?)' to '(.+?)'", message)
        if match:
            return match.group(1), match.group(2)

        # Pattern: "'type1' to 'type2'"
        match = re.search(r"'(.+?)' to '(.+?)'", message)
        if match:
            return match.group(1), match.group(2)

        return "unknown", "unknown"

    @staticmethod
    def _determine_category(message: str, warning_flag: str) -> str:
        """Determine conversion category from message"""
        msg_lower = message.lower()
        flag_lower = warning_flag.lower()

        if 'sign' in msg_lower or 'sign' in flag_lower:
            return "SignChange"
        elif 'float' in msg_lower or 'float' in flag_lower:
            return "Narrowing"
        elif 'shorten' in msg_lower or 'shorten' in flag_lower:
            return "Narrowing"
        elif 'implicit' in msg_lower:
            return "Unknown"
        else:
            return "Unknown"

    @staticmethod
    def _estimate_severity(message: str, warning_flag: str) -> SeverityLevel:
        """Estimate severity based on warning type"""
        msg_lower = message.lower()
        flag_lower = warning_flag.lower()

        # High severity indicators
        if any(keyword in msg_lower for keyword in ['sign', 'unsigned', 'negative']):
            return SeverityLevel.HIGH

        # Medium severity indicators
        if any(keyword in msg_lower for keyword in ['precision', 'float', 'shorten']):
            return SeverityLevel.MEDIUM

        # Default to low
        return SeverityLevel.LOW


class FindingNormalizer:
    """Normalize findings from different tools for comparison"""

    @staticmethod
    def normalize_location(location: Location, base_path: Optional[str] = None) -> Location:
        """Normalize file paths for comparison"""
        file_path = location.file_path

        # Remove base path if provided
        if base_path:
            file_path = file_path.replace(base_path, '').lstrip('/')

        # Normalize path separators
        file_path = file_path.replace('\\', '/')

        return Location(
            file_path=file_path,
            line=location.line,
            column=location.column
        )

    @staticmethod
    def normalize_type(type_str: str) -> str:
        """Normalize type names for comparison"""
        # Remove qualifiers
        type_str = type_str.replace('const ', '')
        type_str = type_str.replace('volatile ', '')
        type_str = type_str.replace('struct ', '')
        type_str = type_str.replace('class ', '')

        # Normalize whitespace
        type_str = ' '.join(type_str.split())

        return type_str.strip()

    @staticmethod
    def are_findings_equivalent(f1: Finding, f2: Finding,
                               location_tolerance: int = 2) -> bool:
        """
        Check if two findings are equivalent

        Args:
            f1: First finding
            f2: Second finding
            location_tolerance: Line number tolerance for matching
        """
        # Must be in same file
        if f1.location.file_path != f2.location.file_path:
            return False

        # Line numbers must be close
        line_diff = abs(f1.location.line - f2.location.line)
        if line_diff > location_tolerance:
            return False

        # Types should match (after normalization)
        source1 = FindingNormalizer.normalize_type(f1.source_type)
        target1 = FindingNormalizer.normalize_type(f1.target_type)
        source2 = FindingNormalizer.normalize_type(f2.source_type)
        target2 = FindingNormalizer.normalize_type(f2.target_type)

        return source1 == source2 and target1 == target2
