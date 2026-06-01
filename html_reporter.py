import json
import os
from pathlib import Path

def generate_html_report(json_data_or_path, output_path):
    """
    Generates a premium, interactive HTML report with charts and search filters.
    
    Args:
        json_data_or_path: Dict of parsed json findings or path to the JSON file
        output_path: Path where the HTML file should be written
    """
    if isinstance(json_data_or_path, (str, Path)):
        with open(json_data_or_path, 'r') as f:
            data = json.load(f)
    else:
        data = json_data_or_path

    findings = data.get('findings', [])
    total_findings = len(findings)
    
    # Calculate statistics
    severities = {"CRITICAL": 0, "HIGH": 0, "MEDIUM": 0, "LOW": 0}
    contexts = {}
    categories = {}
    
    for f in findings:
        sev = f.get('severity', 'LOW')
        severities[sev] = severities.get(sev, 0) + 1
        
        ctx = f.get('context', 'ASSIGNMENT_CONTEXT')
        contexts[ctx] = contexts.get(ctx, 0) + 1
        
        cat = f.get('conversion', {}).get('category', 'Unknown')
        categories[cat] = categories.get(cat, 0) + 1

    # Format findings for JavaScript
    findings_json = json.dumps(findings, indent=2)

    html_content = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ICHA - Static Analysis Report Dashboard</title>
    <link href="https://fonts.googleapis.com/css2?family=Outfit:wght@300;400;500;600;700&family=JetBrains+Mono:wght@400;500&display=swap" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        :root {{
            --bg-color: #0b0f19;
            --card-bg: rgba(17, 24, 39, 0.7);
            --border-color: rgba(255, 255, 255, 0.08);
            --text-main: #f3f4f6;
            --text-dim: #9ca3af;
            --accent-cyan: #06b6d4;
            --accent-blue: #3b82f6;
            --color-critical: #ef4444;
            --color-high: #f97316;
            --color-medium: #eab308;
            --color-low: #22c55e;
            --glass-backdrop: blur(12px);
        }}

        .light-theme {{
            --bg-color: #f8fafc;
            --card-bg: rgba(255, 255, 255, 0.85);
            --border-color: rgba(0, 0, 0, 0.06);
            --text-main: #0f172a;
            --text-dim: #475569;
            --accent-cyan: #0891b2;
            --accent-blue: #2563eb;
            --glass-backdrop: blur(8px);
        }}

        * {{
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            transition: background-color 0.3s, border-color 0.3s;
        }}

        body {{
            font-family: 'Outfit', sans-serif;
            background-color: var(--bg-color);
            color: var(--text-main);
            min-height: 100vh;
            padding: 2rem;
            line-height: 1.5;
        }}

        /* ─── Glassmorphism Layout ────────────────────────────────────────── */

        .dashboard-container {{
            max-width: 1400px;
            margin: 0 auto;
        }}

        header {{
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding-bottom: 2rem;
            border-bottom: 1px solid var(--border-color);
            margin-bottom: 2rem;
        }}

        .logo-section h1 {{
            font-size: 2.2rem;
            font-weight: 700;
            background: linear-gradient(135deg, #06b6d4, #3b82f6);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }}

        .logo-section p {{
            color: var(--text-dim);
            font-size: 0.95rem;
            margin-top: 0.25rem;
        }}

        .theme-btn {{
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            color: var(--text-main);
            padding: 0.6rem 1.2rem;
            border-radius: 9999px;
            cursor: pointer;
            font-family: inherit;
            font-weight: 500;
            display: flex;
            align-items: center;
            gap: 0.5rem;
            backdrop-filter: var(--glass-backdrop);
        }}

        .theme-btn:hover {{
            border-color: var(--accent-cyan);
        }}

        /* Stats Cards */
        .stats-grid {{
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
            gap: 1.5rem;
            margin-bottom: 2rem;
        }}

        .stat-card {{
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            backdrop-filter: var(--glass-backdrop);
            border-radius: 16px;
            padding: 1.5rem;
            display: flex;
            flex-direction: column;
            position: relative;
            overflow: hidden;
        }}

        .stat-card::before {{
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 4px;
            height: 100%;
        }}

        .stat-card.total::before {{ background: var(--accent-cyan); }}
        .stat-card.critical::before {{ background: var(--color-critical); }}
        .stat-card.high::before {{ background: var(--color-high); }}
        .stat-card.medium::before {{ background: var(--color-medium); }}
        .stat-card.low::before {{ background: var(--color-low); }}

        .stat-label {{
            font-size: 0.9rem;
            color: var(--text-dim);
            text-transform: uppercase;
            letter-spacing: 0.05em;
        }}

        .stat-val {{
            font-size: 2.2rem;
            font-weight: 700;
            margin-top: 0.5rem;
        }}

        /* Visualization Charts */
        .visualizations {{
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 2rem;
            margin-bottom: 3rem;
        }}

        @media (max-width: 900px) {{
            .visualizations {{
                grid-template-columns: 1fr;
            }}
        }}

        .chart-card {{
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            backdrop-filter: var(--glass-backdrop);
            border-radius: 20px;
            padding: 1.5rem;
            min-height: 320px;
        }}

        .chart-card h3 {{
            font-size: 1.15rem;
            font-weight: 600;
            margin-bottom: 1.5rem;
            color: var(--text-main);
            border-bottom: 1px solid var(--border-color);
            padding-bottom: 0.75rem;
        }}

        .chart-wrapper {{
            position: relative;
            height: 240px;
            width: 100%;
        }}

        /* Filter Section */
        .filter-section {{
            display: flex;
            gap: 1rem;
            margin-bottom: 2rem;
            flex-wrap: wrap;
        }}

        .search-bar {{
            flex: 1;
            min-width: 280px;
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            border-radius: 12px;
            padding: 0.8rem 1.2rem;
            color: var(--text-main);
            font-family: inherit;
            font-size: 1rem;
            backdrop-filter: var(--glass-backdrop);
        }}

        .search-bar:focus {{
            outline: none;
            border-color: var(--accent-cyan);
        }}

        .filter-select {{
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            color: var(--text-main);
            padding: 0.8rem 1.2rem;
            border-radius: 12px;
            font-family: inherit;
            font-size: 0.95rem;
            cursor: pointer;
            backdrop-filter: var(--glass-backdrop);
        }}

        /* Finding Cards */
        .findings-list {{
            display: flex;
            flex-direction: column;
            gap: 1rem;
        }}

        .finding-card {{
            background: var(--card-bg);
            border: 1px solid var(--border-color);
            backdrop-filter: var(--glass-backdrop);
            border-radius: 16px;
            overflow: hidden;
            border-left: 4px solid var(--accent-blue);
        }}

        .finding-card.critical {{ border-left-color: var(--color-critical); }}
        .finding-card.high {{ border-left-color: var(--color-high); }}
        .finding-card.medium {{ border-left-color: var(--color-medium); }}
        .finding-card.low {{ border-left-color: var(--color-low); }}

        .card-header {{
            padding: 1.2rem 1.5rem;
            cursor: pointer;
            display: flex;
            justify-content: space-between;
            align-items: center;
            user-select: none;
        }}

        .card-header-main {{
            display: flex;
            align-items: center;
            gap: 1rem;
            flex-wrap: wrap;
        }}

        .badge {{
            padding: 0.25rem 0.75rem;
            border-radius: 9999px;
            font-size: 0.75rem;
            font-weight: 700;
            letter-spacing: 0.05em;
        }}

        .badge.critical {{ background: rgba(239, 68, 68, 0.15); color: #f87171; border: 1px solid rgba(239, 68, 68, 0.3); }}
        .badge.high {{ background: rgba(249, 115, 22, 0.15); color: #fb923c; border: 1px solid rgba(249, 115, 22, 0.3); }}
        .badge.medium {{ background: rgba(234, 179, 8, 0.15); color: #facc15; border: 1px solid rgba(234, 179, 8, 0.3); }}
        .badge.low {{ background: rgba(34, 197, 94, 0.15); color: #4ade80; border: 1px solid rgba(34, 197, 94, 0.3); }}

        .finding-title {{
            font-weight: 600;
            font-size: 1.05rem;
        }}

        .finding-loc {{
            font-family: 'JetBrains Mono', monospace;
            font-size: 0.85rem;
            color: var(--text-dim);
            background: rgba(255, 255, 255, 0.04);
            padding: 0.15rem 0.5rem;
            border-radius: 6px;
        }}

        .card-header-side {{
            display: flex;
            align-items: center;
            gap: 1rem;
        }}

        .score-indicator {{
            font-weight: 700;
            font-size: 0.95rem;
        }}

        .arrow-icon {{
            font-size: 1.2rem;
            transition: transform 0.2s;
        }}

        .finding-card.expanded .arrow-icon {{
            transform: rotate(180deg);
        }}

        .card-body {{
            max-height: 0;
            overflow: hidden;
            transition: max-height 0.3s cubic-bezier(0, 1, 0, 1);
            border-top: 1px solid transparent;
        }}

        .finding-card.expanded .card-body {{
            max-height: 2000px;
            transition: max-height 0.3s cubic-bezier(1, 0, 1, 0);
            border-top-color: var(--border-color);
        }}

        .body-content {{
            padding: 1.5rem;
            display: grid;
            grid-template-columns: 1fr 1fr;
            gap: 1.5rem;
        }}

        @media (max-width: 768px) {{
            .body-content {{
                grid-template-columns: 1fr;
            }}
        }}

        .panel-heading {{
            font-size: 0.9rem;
            text-transform: uppercase;
            letter-spacing: 0.05em;
            color: var(--text-dim);
            margin-bottom: 0.5rem;
            font-weight: 600;
        }}

        .panel-desc {{
            background: rgba(0, 0, 0, 0.15);
            border: 1px solid var(--border-color);
            border-radius: 12px;
            padding: 1rem;
            font-size: 0.95rem;
        }}

        .fix-box {{
            background: rgba(6, 182, 212, 0.04);
            border: 1px dashed rgba(6, 182, 212, 0.3);
        }}

        .breakdown-row {{
            display: flex;
            justify-content: space-between;
            padding: 0.4rem 0;
            border-bottom: 1px solid var(--border-color);
            font-size: 0.9rem;
        }}

        .breakdown-row:last-child {{
            border-bottom: none;
        }}

        /* Code Block */
        code {{
            font-family: 'JetBrains Mono', monospace;
            background: rgba(0, 0, 0, 0.2);
            color: #ff79c6;
            padding: 0.1rem 0.3rem;
            border-radius: 4px;
            font-size: 0.85rem;
        }}

        pre code {{
            display: block;
            padding: 0.8rem;
            background: rgba(0, 0, 0, 0.3);
            border-radius: 8px;
            color: #f8f8f2;
            overflow-x: auto;
            border: 1px solid var(--border-color);
            white-space: pre-wrap;
        }}
    </style>
</head>
<body>
    <div class="dashboard-container">
        <header>
            <div class="logo-section">
                <h1>⚡ ICHA Analysis Report</h1>
                <p>Implicit Conversion Hazard Analyzer • Interactive Analytics</p>
            </div>
            <button class="theme-btn" id="theme-toggle">
                <span>🌓 Toggle Theme</span>
            </button>
        </header>

        <!-- Stats Grid -->
        <div class="stats-grid">
            <div class="stat-card total">
                <span class="stat-label">Total Findings</span>
                <span class="stat-val" id="count-total">{total_findings}</span>
            </div>
            <div class="stat-card critical">
                <span class="stat-label">Critical Risks</span>
                <span class="stat-val" id="count-critical">{severities["CRITICAL"]}</span>
            </div>
            <div class="stat-card high">
                <span class="stat-label">High Risks</span>
                <span class="stat-val" id="count-high">{severities["HIGH"]}</span>
            </div>
            <div class="stat-card medium">
                <span class="stat-label">Medium Risks</span>
                <span class="stat-val" id="count-medium">{severities["MEDIUM"]}</span>
            </div>
            <div class="stat-card low">
                <span class="stat-label">Low Risks</span>
                <span class="stat-val" id="count-low">{severities["LOW"]}</span>
            </div>
        </div>

        <!-- Visualizations Grid -->
        <div class="visualizations">
            <div class="chart-card">
                <h3>Severity Distribution</h3>
                <div class="chart-wrapper">
                    <canvas id="severityChart"></canvas>
                </div>
            </div>
            <div class="chart-card">
                <h3>Context Breakdown</h3>
                <div class="chart-wrapper">
                    <canvas id="contextChart"></canvas>
                </div>
            </div>
        </div>

        <!-- Filter Controls -->
        <div class="filter-section">
            <input type="text" id="search-input" class="search-bar" placeholder="Search by file path, types or explanation...">
            <select id="severity-filter" class="filter-select">
                <option value="ALL">All Severities</option>
                <option value="CRITICAL">Critical Only</option>
                <option value="HIGH">High & Above</option>
                <option value="MEDIUM">Medium & Above</option>
                <option value="LOW">Low Only</option>
            </select>
            <select id="context-filter" class="filter-select">
                <option value="ALL">All Contexts</option>
                {"".join(f'<option value="{ctx}">{ctx}</option>' for ctx in contexts.keys())}
            </select>
        </div>

        <!-- Findings List -->
        <div class="findings-list" id="findings-container">
            <!-- Dynamic findings here -->
        </div>
    </div>

    <script>
        // Data injected from python
        const findings = {findings_json};

        // Theme management
        const toggleBtn = document.getElementById('theme-toggle');
        toggleBtn.addEventListener('click', () => {{
            document.body.classList.toggle('light-theme');
        }});

        // Generate Charts
        const severityCtx = document.getElementById('severityChart').getContext('2d');
        new Chart(severityCtx, {{
            type: 'doughnut',
            data: {{
                labels: ['Critical', 'High', 'Medium', 'Low'],
                datasets: [{{
                    data: [{severities["CRITICAL"]}, {severities["HIGH"]}, {severities["MEDIUM"]}, {severities["LOW"]}],
                    backgroundColor: ['#ef4444', '#f97316', '#eab308', '#22c55e'],
                    borderColor: 'transparent'
                }}]
            }},
            options: {{
                responsive: true,
                maintainAspectRatio: false,
                plugins: {{
                    legend: {{ position: 'right', labels: {{ color: '#9ca3af' }} }}
                }}
            }}
        }});

        const contextLabels = {list(contexts.keys())};
        const contextData = {list(contexts.values())};
        const contextCtx = document.getElementById('contextChart').getContext('2d');
        new Chart(contextCtx, {{
            type: 'bar',
            data: {{
                labels: contextLabels,
                datasets: [{{
                    label: 'Findings Count',
                    data: contextData,
                    backgroundColor: '#3b82f6',
                    borderRadius: 6
                }}]
            }},
            options: {{
                responsive: true,
                maintainAspectRatio: false,
                scales: {{
                    y: {{ grid: {{ color: 'rgba(255,255,255,0.05)' }}, ticks: {{ color: '#9ca3af' }} }},
                    x: {{ grid: {{ display: false }}, ticks: {{ color: '#9ca3af' }} }}
                }},
                plugins: {{
                    legend: {{ display: false }}
                }}
            }}
        }});

        // Render findings
        const container = document.getElementById('findings-container');

        function renderFindings(filtered) {{
            container.innerHTML = '';
            if (filtered.length === 0) {{
                container.innerHTML = '<div class="stat-card" style="align-items: center; justify-content: center; padding: 3rem; color: var(--text-dim);">No findings match the active filters.</div>';
                return;
            }}

            filtered.forEach(f => {{
                const card = document.createElement('div');
                card.className = `finding-card ${{f.severity.toLowerCase()}}`;
                
                const relativeFile = f.location.file.split('/').pop();
                
                card.innerHTML = `
                    <div class="card-header" onclick="this.parentElement.classList.toggle('expanded')">
                        <div class="card-header-main">
                            <span class="badge ${{f.severity.toLowerCase()}}">${{f.severity}}</span>
                            <span class="finding-title">Implicit Conversion: <code>${{f.conversion.source}}</code> ➜ <code>${{f.conversion.target}}</code></span>
                            <span class="finding-loc" title="${{f.location.file}}">${{relativeFile}}:${{f.location.line}}</span>
                        </div>
                        <div class="card-header-side">
                            <span class="score-indicator" style="color: ${{f.severity == 'CRITICAL' ? '#ef4444' : f.severity == 'HIGH' ? '#f97316' : f.severity == 'MEDIUM' ? '#eab308' : '#22c55e'}}">
                                Risk: ${{f.risk.finalRisk}}/100
                            </span>
                            <span class="arrow-icon">▼</span>
                        </div>
                    </div>
                    <div class="card-body">
                        <div class="body-content">
                            <div>
                                <div class="panel-heading">Context Intelligence</div>
                                <div class="panel-desc">
                                    <p style="margin-bottom: 0.5rem;"><strong>Primary Context:</strong> <code>${{f.context}}</code></p>
                                    <p>${{f.explanation}}</p>
                                </div>
                                <div class="panel-heading" style="margin-top: 1rem;">Risk Assessment Details</div>
                                <div class="panel-desc">
                                    <div class="breakdown-row"><span>Conversion Type Risk</span> <span>${{f.risk.conversionRisk}} / 35</span></div>
                                    <div class="breakdown-row"><span>Semantic Context Factor</span> <span>${{f.risk.contextRisk}} / 30</span></div>
                                    <div class="breakdown-row"><span>Value Impact Factor</span> <span>${{f.risk.impactRisk}} / 30</span></div>
                                    <div class="breakdown-row"><span>Data Propagation Weight</span> <span>${{f.risk.propagationRisk}} / 5</span></div>
                                </div>
                            </div>
                            <div>
                                <div class="panel-heading">Suggested Resolution</div>
                                <div class="panel-desc fix-box">
                                    <p style="margin-bottom: 0.75rem; line-height: 1.45;">${{f.fix}}</p>
                                    <pre><code>// Example Safe Fix Action\\n// Use explicit cast with range validation where needed\\nstatic_cast<${{f.conversion.target}}>(...)</code></pre>
                                </div>
                            </div>
                        </div>
                    </div>
                `;
                container.appendChild(card);
            }});
        }}

        // Filters handler
        const searchInput = document.getElementById('search-input');
        const severityFilter = document.getElementById('severity-filter');
        const contextFilter = document.getElementById('context-filter');

        function applyFilters() {{
            const query = searchInput.value.toLowerCase();
            const sevVal = severityFilter.value;
            const ctxVal = contextFilter.value;

            const filtered = findings.filter(f => {{
                // Search match
                const matchSearch = f.location.file.toLowerCase().includes(query) ||
                                    f.conversion.source.toLowerCase().includes(query) ||
                                    f.conversion.target.toLowerCase().includes(query) ||
                                    f.explanation.toLowerCase().includes(query);

                // Severity match
                let matchSev = true;
                if (sevVal === 'CRITICAL') matchSev = f.severity === 'CRITICAL';
                else if (sevVal === 'HIGH') matchSev = ['CRITICAL', 'HIGH'].includes(f.severity);
                else if (sevVal === 'MEDIUM') matchSev = ['CRITICAL', 'HIGH', 'MEDIUM'].includes(f.severity);
                else if (sevVal === 'LOW') matchSev = f.severity === 'LOW';

                // Context match
                const matchCtx = ctxVal === 'ALL' || f.context === ctxVal;

                return matchSearch && matchSev && matchCtx;
            }});

            renderFindings(filtered);
        }}

        searchInput.addEventListener('input', applyFilters);
        severityFilter.addEventListener('change', applyFilters);
        contextFilter.addEventListener('change', applyFilters);

        // Initial render
        renderFindings(findings);
    </script>
</body>
</html>
"""

    with open(output_path, 'w') as f:
        f.write(html_content)

    print(f"✓ Premium HTML report generated successfully at: {output_path}")
