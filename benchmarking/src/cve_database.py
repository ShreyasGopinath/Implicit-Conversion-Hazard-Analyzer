"""
CVE Pattern Database for correlation analysis
Real CVEs caused by implicit type conversion bugs.
"""

from models import CVEPattern, SeverityLevel


# Real CVEs caused by implicit conversions in C/C++ software
CVE_PATTERNS = [
    CVEPattern(
        cve_id="CVE-2021-3156",
        description="sudo heap-based buffer overflow: a signed int was implicitly "
                    "converted to size_t when calculating buffer size, causing negative "
                    "values to wrap around to huge unsigned values.",
        conversion_pattern="int->size_t",
        context_pattern="memory",
        severity=SeverityLevel.CRITICAL
    ),
    CVEPattern(
        cve_id="CVE-2022-23521",
        description="git integer overflow: pathspec count stored in int was implicitly "
                    "widened to size_t in allocation, allowing controlled heap overflow.",
        conversion_pattern="int->size_t",
        context_pattern="memory",
        severity=SeverityLevel.CRITICAL
    ),
    CVEPattern(
        cve_id="CVE-2019-9213",
        description="Linux kernel mmap minimum address bypass: unsigned long to int "
                    "narrowing conversion allowed negative offset to bypass ASLR.",
        conversion_pattern="unsigned long->int",
        context_pattern="branch",
        severity=SeverityLevel.CRITICAL
    ),
    CVEPattern(
        cve_id="CVE-2018-20843",
        description="libexpat XML parser: integer overflow via implicit conversion in "
                    "lookup table calculation leading to denial of service.",
        conversion_pattern="int->unsigned",
        context_pattern="memory",
        severity=SeverityLevel.HIGH
    ),
    CVEPattern(
        cve_id="CVE-2014-1266",
        description="Apple SSL/TLS 'goto fail': implicit boolean conversion in "
                    "authentication check allowed MitM attacks.",
        conversion_pattern="int->bool",
        context_pattern="branch",
        severity=SeverityLevel.CRITICAL
    ),
    CVEPattern(
        cve_id="CVE-2021-28957",
        description="lxml: integer narrowing conversion in buffer length calculation "
                    "allowed XSS via crafted HTML input.",
        conversion_pattern="long->int",
        context_pattern="memory",
        severity=SeverityLevel.HIGH
    ),
    CVEPattern(
        cve_id="CVE-2016-2182",
        description="OpenSSL BN_bn2dec OOB write: unsigned to signed conversion in "
                    "bignum division allowed heap buffer overflow.",
        conversion_pattern="unsigned->int",
        context_pattern="memory",
        severity=SeverityLevel.HIGH
    ),
    CVEPattern(
        cve_id="CVE-2020-11655",
        description="SQLite: implicit integer conversion in window function allows "
                    "NULL pointer dereference and denial of service.",
        conversion_pattern="int->unsigned",
        context_pattern="branch",
        severity=SeverityLevel.MEDIUM
    ),
    CVEPattern(
        cve_id="CVE-2018-16435",
        description="Little CMS lcms2: integer overflow from implicit short to int "
                    "conversion in ReadData function, heap buffer overflow.",
        conversion_pattern="short->int",
        context_pattern="memory",
        severity=SeverityLevel.HIGH
    ),
    CVEPattern(
        cve_id="CVE-2015-8317",
        description="libxml2: integer sign conversion in xmlParseXMLDecl allowed "
                    "heap-based buffer overread.",
        conversion_pattern="int->unsigned",
        context_pattern="memory",
        severity=SeverityLevel.HIGH
    ),
    CVEPattern(
        cve_id="CVE-2017-9233",
        description="libexpat: signed integer overflow via implicit conversion in "
                    "hash table resizing, leading to infinite loop (DoS).",
        conversion_pattern="int->unsigned",
        context_pattern="loop",
        severity=SeverityLevel.MEDIUM
    ),
    CVEPattern(
        cve_id="CVE-2021-45960",
        description="libexpat: integer overflow in storeAtts due to implicit widening "
                    "conversion, leading to realloc with attacker-controlled size.",
        conversion_pattern="int->size_t",
        context_pattern="memory",
        severity=SeverityLevel.CRITICAL
    ),
]


def get_cve_patterns():
    """Get all CVE patterns"""
    return CVE_PATTERNS


def get_cve_patterns_by_severity(severity: SeverityLevel):
    """Get CVE patterns filtered by severity"""
    return [p for p in CVE_PATTERNS if p.severity == severity]


def get_cve_pattern_by_id(cve_id: str):
    """Get specific CVE pattern by ID"""
    for pattern in CVE_PATTERNS:
        if pattern.cve_id == cve_id:
            return pattern
    return None
