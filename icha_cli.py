#!/usr/bin/env python3
"""
Implicit Conversion Hazard Analyzer for C/C++ Advanced Terminal User Interface
Premium interactive CLI for the Implicit Conversion Hazard Analyzer.
"""

import sys, os, subprocess, json, time, glob
from datetime import datetime
from prompt_toolkit import PromptSession
from prompt_toolkit.completion import WordCompleter, PathCompleter, merge_completers
from prompt_toolkit.styles import Style
from prompt_toolkit.formatted_text import HTML
from rich.console import Console
from rich.panel import Panel
from rich.table import Table
from rich.tree import Tree
from rich.columns import Columns
from rich.markdown import Markdown
from rich.progress import Progress, SpinnerColumn, TextColumn, BarColumn
from rich.layout import Layout
from rich.text import Text
from rich.rule import Rule
from rich.bar import Bar
from rich import box

console = Console()

# ─── Styling ──────────────────────────────────────────────────────────────────

SEVERITY_STYLES = {
    "CRITICAL": ("bold white on red", "🔴", "red"),
    "HIGH":     ("bold red", "🟠", "dark_orange"),
    "MEDIUM":   ("bold yellow", "🟡", "yellow"),
    "LOW":      ("bold green", "🟢", "green"),
}

CONTEXT_ICONS = {
    "MEMORY_CONTEXT":     "🧠",
    "LOOP_CONTEXT":       "🔁",
    "BRANCH_CONTEXT":     "🔀",
    "RETURN_CONTEXT":     "↩️ ",
    "API_CONTEXT":        "🔌",
    "ARITHMETIC_CONTEXT": "➕",
    "ASSIGNMENT_CONTEXT": "📝",
}

COMMANDS = ['/help', '/build', '/test', '/analyze', '/debug', '/inspect',
            '/status', '/clear', '/exit', '/quit', '/report', '/benchmark', '/compare', '/html']

cmd_completer = WordCompleter(COMMANDS, ignore_case=True)
path_completer = PathCompleter(expanduser=True)
completer = merge_completers([cmd_completer, path_completer])

prompt_style = Style.from_dict({
    'prompt': 'bold ansibrightcyan',
    'rprompt': 'fg:ansigray',
})

# ─── Welcome Banner ──────────────────────────────────────────────────────────

def print_welcome():
    banner = Text()
    banner.append("╔══════════════════════════════════════════════════════════════╗\n", style="bold cyan")
    banner.append("║", style="bold cyan")
    banner.append("          ⚡ Implicit Conversion Hazard Analyzer for C/C++ ⚡           ", style="bold white")
    banner.append("║\n", style="bold cyan")
    banner.append("║", style="bold cyan")
    banner.append("      Implicit Conversion Hazard Analyzer  v1.0.0          ", style="dim white")
    banner.append("║\n", style="bold cyan")
    banner.append("╚══════════════════════════════════════════════════════════════╝", style="bold cyan")
    console.print(banner)
    console.print()
    console.print("  Type [bold cyan]/help[/] for commands  •  Tab for autocomplete  •  [dim]Ctrl+D to exit[/dim]")
    console.print()

# ─── Help ─────────────────────────────────────────────────────────────────────

def print_help():
    console.print(Rule("[bold cyan]Available Commands[/]"))
    table = Table(box=box.ROUNDED, border_style="cyan", show_edge=True, pad_edge=True)
    table.add_column("Command", style="bold cyan", width=22)
    table.add_column("Description", style="white")
    table.add_column("Example", style="dim")

    table.add_row("/analyze <file>", "Run full analysis with detailed visualization", "/analyze my_code.c")
    table.add_row("/debug <file>",   "Analyze with AST parent chain debug view",      "/debug my_code.c")
    table.add_row("/inspect <file>", "Show detailed per-finding breakdown panels",     "/inspect my_code.c")
    table.add_row("/report <file>",  "Generate the full CLI report (raw output)",      "/report my_code.c")
    table.add_row("/html <file>",    "Generate premium interactive HTML dashboard",   "/html my_code.c")
    table.add_row("/benchmark",      "Run full OSS evaluation & CVE correlation",      "")
    table.add_row("/compare",        "Show Implicit Conversion Hazard Analyzer vs Clang baseline comparison",         "")
    table.add_row("/test",           "Run all validation & risk differentiation tests","")
    table.add_row("/build",          "Recompile the Implicit Conversion Hazard Analyzer for C/C++ engine",                  "")
    table.add_row("/status",         "Show project build & binary status",             "")
    table.add_row("/clear",          "Clear the terminal screen",                      "")
    table.add_row("/help",           "Show this help menu",                            "")
    table.add_row("/exit",           "Exit the interactive terminal",                  "")

    console.print(table)
    console.print()

# ─── Status ───────────────────────────────────────────────────────────────────

def handle_status():
    console.print(Rule("[bold cyan]Project Status[/]"))
    binary = os.path.exists("build/src/icha")
    venv = os.path.exists(".venv")
    test_files = glob.glob("tests/validation/*.c")

    table = Table(box=box.SIMPLE_HEAVY, border_style="cyan")
    table.add_column("Component", style="bold")
    table.add_column("Status")
    table.add_row("Implicit Conversion Hazard Analyzer Binary", "[bold green]✅ Ready[/]" if binary else "[bold red]❌ Not Built[/]")
    table.add_row("Python Env", "[bold green]✅ Active[/]" if venv else "[dim]Not created[/dim]")
    table.add_row("Validation Tests", f"[bold]{len(test_files)}[/] test files found")
    table.add_row("Timestamp", datetime.now().strftime("%Y-%m-%d %H:%M:%S"))
    console.print(table)
    console.print()

# ─── Build ────────────────────────────────────────────────────────────────────

def handle_build():
    console.print(Rule("[bold cyan]Building Implicit Conversion Hazard Analyzer Engine[/]"))
    with Progress(SpinnerColumn("dots"), TextColumn("[bold cyan]{task.description}"), console=console, transient=True) as p:
        t = p.add_task("Compiling C++ source via CMake...", total=None)
        result = subprocess.run(["make", "-C", "build"], capture_output=True, text=True)
        p.update(t, completed=True)

    if result.returncode == 0:
        console.print(Panel("[bold green]✅ Build Completed Successfully![/]", border_style="green"))
    else:
        console.print(Panel("[bold red]❌ Build Failed[/]", border_style="red"))
        console.print(result.stderr)
    console.print()

# ─── Test ─────────────────────────────────────────────────────────────────────

def handle_test():
    console.print(Rule("[bold cyan]Running Test Suites[/]"))

    scripts = [
        ("./scripts/run_validation_tests.sh",       "Context Validation Suite",    "6 context-specific test files"),
        ("./scripts/test_risk_differentiation.sh",  "Risk Differentiation Suite",  "Verifies risk scores differ by context"),
    ]

    results = []
    for script, name, desc in scripts:
        console.print(f"\n[bold cyan]▶ {name}[/bold cyan] — [dim]{desc}[/dim]\n")
        result = subprocess.run([script], capture_output=True, text=True)
        passed = result.returncode == 0
        results.append((name, passed, result))

        # Always show the full detailed output
        if result.stdout.strip():
            console.print(Panel(result.stdout.strip(), title=f"[bold green]{name} Output[/bold green]" if passed else f"[bold red]{name} Output[/bold red]", border_style="green" if passed else "red", box=box.ROUNDED))
        if not passed and result.stderr.strip():
            console.print(f"[red]{result.stderr[-500:]}[/red]")

    console.print()
    table = Table(box=box.ROUNDED, border_style="cyan", title="Test Results Summary")
    table.add_column("Suite", style="bold")
    table.add_column("Result", justify="center")
    for name, passed, _ in results:
        table.add_row(name, "[bold green]✅ PASSED[/]" if passed else "[bold red]❌ FAILED[/]")
    console.print(table)
    console.print()

# ─── Core Analysis Runner ────────────────────────────────────────────────────

def run_icha(file_path, extra_flags=None):
    """Run the Implicit Conversion Hazard Analyzer binary and return parsed JSON or None."""
    if not os.path.exists(file_path):
        console.print(f"[bold red]Error: File '{file_path}' not found.[/]")
        return None

    cmd = ["./build/src/icha", file_path]
    if extra_flags:
        cmd.extend(extra_flags)
    cmd.extend(["--icha-format=json", "--"])

    with Progress(SpinnerColumn("dots"), TextColumn("[cyan]Running Implicit Conversion Hazard Analyzer engine...[/]"), console=console, transient=True) as p:
        t = p.add_task("Analyzing...", total=None)
        result = subprocess.run(cmd, capture_output=True, text=True)
        p.update(t, completed=True)

    output = result.stdout
    start = output.find('{')
    if start != -1:
        try:
            return json.loads(output[start:])
        except json.JSONDecodeError:
            pass

    if result.returncode == 0 and start == -1:
        console.print(Panel("[bold green]✅ No implicit conversion hazards detected![/]", border_style="green"))
    else:
        console.print("[bold red]❌ Analysis failed or could not parse output.[/]")
        if result.stderr:
            console.print(result.stderr[:800])
    return None

# ─── /analyze — Summary Table + Charts ────────────────────────────────────────

def handle_analyze(args):
    if not args:
        console.print("[red]Usage: /analyze <file.c>[/]")
        return

    report = run_icha(args[0])
    if not report:
        return

    findings = report.get("findings", [])
    if not findings:
        return

    console.print(Rule(f"[bold cyan]Analysis: {args[0]}[/]"))

    # ── Severity Summary ──
    counts = {"CRITICAL": 0, "HIGH": 0, "MEDIUM": 0, "LOW": 0}
    for f in findings:
        sev = f.get("severity", "LOW")
        counts[sev] = counts.get(sev, 0) + 1

    summary = Table(box=box.SIMPLE_HEAVY, border_style="cyan", title="Severity Distribution")
    summary.add_column("Severity", style="bold", width=12)
    summary.add_column("Count", justify="center", width=6)
    summary.add_column("Bar", width=30)
    max_c = max(counts.values()) if counts.values() else 1
    for sev, cnt in counts.items():
        _, icon, color = SEVERITY_STYLES[sev]
        bar_len = int((cnt / max_c) * 25) if max_c > 0 else 0
        bar_str = f"[{color}]{'█' * bar_len}{'░' * (25 - bar_len)}[/]"
        summary.add_row(f"{icon} {sev}", str(cnt), bar_str)
    console.print(summary)
    console.print()

    # ── Context Distribution ──
    ctx_counts = {}
    for f in findings:
        ctx = f.get("context", "UNKNOWN")
        ctx_counts[ctx] = ctx_counts.get(ctx, 0) + 1

    ctx_table = Table(box=box.SIMPLE_HEAVY, border_style="cyan", title="Context Distribution")
    ctx_table.add_column("Context", style="bold", width=22)
    ctx_table.add_column("Count", justify="center", width=6)
    ctx_table.add_column("Bar", width=30)
    max_ctx = max(ctx_counts.values()) if ctx_counts.values() else 1
    for ctx, cnt in sorted(ctx_counts.items(), key=lambda x: -x[1]):
        icon = CONTEXT_ICONS.get(ctx, "❓")
        bar_len = int((cnt / max_ctx) * 25) if max_ctx > 0 else 0
        bar_str = f"[cyan]{'█' * bar_len}{'░' * (25 - bar_len)}[/]"
        ctx_table.add_row(f"{icon} {ctx}", str(cnt), bar_str)
    console.print(ctx_table)
    console.print()

    # ── Findings Table ──
    table = Table(box=box.ROUNDED, border_style="dim", title=f"All Findings ({len(findings)})", show_lines=True)
    table.add_column("#", style="dim", width=3, justify="right")
    table.add_column("Location", style="dim", width=10)
    table.add_column("Conversion", width=20)
    table.add_column("Category", width=10)
    table.add_column("Context", style="cyan", width=20)
    table.add_column("Risk", justify="center", width=12)
    table.add_column("Severity", justify="center", width=10)

    for i, f in enumerate(findings, 1):
        loc = f.get("location", {})
        conv = f.get("conversion", {})
        risk = f.get("risk", {})
        sev = f.get("severity", "LOW")
        score = risk.get("finalRisk", 0)
        sev_style, sev_icon, _ = SEVERITY_STYLES.get(sev, ("", "", ""))

        score_bar_len = int(score / 100 * 8)
        score_color = "green" if score < 40 else ("yellow" if score < 60 else "red")
        score_vis = f"[{score_color}]{'█' * score_bar_len}{'░' * (8 - score_bar_len)}[/] {score}"

        table.add_row(
            str(i),
            f"L{loc.get('line','?')}:{loc.get('column','?')}",
            f"{conv.get('source','?')} → {conv.get('target','?')}",
            f.get("conversion", {}).get("category", "?"),
            f"{CONTEXT_ICONS.get(f.get('context',''), '❓')} {f.get('context', '?')}",
            score_vis,
            f"[{sev_style}]{sev_icon} {sev}[/]",
        )

    console.print(table)
    console.print()

# ─── /inspect — Detailed Per-Finding Panels ───────────────────────────────────

def handle_inspect(args):
    if not args:
        console.print("[red]Usage: /inspect <file.c>[/]")
        return

    report = run_icha(args[0])
    if not report:
        return

    findings = report.get("findings", [])
    if not findings:
        return

    console.print(Rule(f"[bold cyan]Detailed Inspection: {args[0]}[/]"))
    console.print(f"  Showing [bold]{len(findings)}[/] findings with full risk breakdown\n")

    for i, f in enumerate(findings, 1):
        loc = f.get("location", {})
        conv = f.get("conversion", {})
        risk = f.get("risk", {})
        sev = f.get("severity", "LOW")
        sev_style, sev_icon, sev_color = SEVERITY_STYLES.get(sev, ("", "", "white"))

        # Risk breakdown tree
        tree = Tree(f"[bold]Risk Breakdown[/] — Total: [bold]{risk.get('finalRisk', 0)}[/]/100")
        tree.add(f"Conversion Risk:  [bold]{risk.get('conversionRisk', 0)}[/]")
        tree.add(f"Context Risk:     [bold]{risk.get('contextRisk', 0)}[/]")
        tree.add(f"Impact Risk:      [bold]{risk.get('impactRisk', 0)}[/]")
        tree.add(f"Propagation Risk: [bold]{risk.get('propagationRisk', 0)}[/]")

        content = Text()
        content.append(f"Conversion: ", style="dim")
        content.append(f"{conv.get('source','?')} → {conv.get('target','?')}\n", style="bold")
        content.append(f"Category:   ", style="dim")
        content.append(f"{conv.get('category', '?')}\n", style="bold")
        content.append(f"Context:    ", style="dim")
        ctx = f.get("context", "?")
        content.append(f"{CONTEXT_ICONS.get(ctx, '')} {ctx}\n", style="bold cyan")

        loc_str = f"{os.path.basename(loc.get('file', '?'))}:{loc.get('line','?')}:{loc.get('column','?')}"
        title = f"{sev_icon} [{sev_style}]Finding #{i} — {sev}[/]  [dim]({loc_str})[/dim]"

        panel_content = content
        console.print(Panel(panel_content, title=title, border_style=sev_color, padding=(0, 2)))
        console.print(tree)

        # Fix suggestion
        fix = f.get("fix", {})
        if fix and fix.get("description"):
            fix_text = Text()
            fix_text.append("  💡 Fix: ", style="bold green")
            fix_text.append(fix.get("description", ""))
            console.print(fix_text)
            steps = fix.get("steps", [])
            for si, step in enumerate(steps, 1):
                console.print(f"      {si}. {step}")

        console.print()

# ─── /debug — AST Parent Chain View ──────────────────────────────────────────

def handle_debug(args):
    if not args:
        console.print("[red]Usage: /debug <file.c>[/]")
        return

    file_path = args[0]
    if not os.path.exists(file_path):
        console.print(f"[bold red]Error: File '{file_path}' not found.[/]")
        return

    console.print(Rule(f"[bold cyan]AST Debug: {file_path}[/]"))

    cmd = ["./build/src/icha", file_path, "-icha-debug-context", "--"]
    with Progress(SpinnerColumn("dots"), TextColumn("[cyan]Running debug analysis...[/]"), console=console, transient=True) as p:
        t = p.add_task("Analyzing...", total=None)
        result = subprocess.run(cmd, capture_output=True, text=True)
        p.update(t, completed=True)

    output = result.stdout
    # Parse the debug blocks
    blocks = output.split("========================================")
    debug_blocks = []
    i = 0
    while i < len(blocks):
        block = blocks[i].strip()
        if block.startswith("DEBUG: AST Parent Chain") and i + 1 < len(blocks):
            debug_blocks.append(blocks[i + 1].strip())
            i += 2
        else:
            i += 1

    if not debug_blocks:
        console.print("[yellow]No debug blocks found. Showing raw output:[/]")
        console.print(output[:3000])
        return

    console.print(f"  Found [bold]{len(debug_blocks)}[/] implicit conversions with AST chains\n")

    for idx, block in enumerate(debug_blocks, 1):
        lines = block.strip().split('\n')
        loc_line = ""
        conv_line = ""
        chain_lines = []
        context_lines = []
        in_chain = False
        in_context = False

        for line in lines:
            line = line.strip()
            if line.startswith("Location:"):
                loc_line = line.replace("Location: ", "")
            elif line.startswith("Conversion:"):
                conv_line = line.replace("Conversion: ", "")
            elif line == "AST Parent Chain:":
                in_chain = True
                in_context = False
            elif line == "Detected Contexts:":
                in_chain = False
                in_context = True
            elif in_chain and line:
                chain_lines.append(line)
            elif in_context and line:
                context_lines.append(line)

        # Build AST tree visualization
        tree = Tree(f"[bold cyan]#{idx}[/] [dim]{loc_line}[/]  [bold]{conv_line}[/]")
        ast_branch = tree.add("[bold]AST Parent Chain[/]")
        for cl in chain_lines:
            node_name = cl.replace("-> ", "").strip()
            # Color known node types
            if "Stmt" in node_name or node_name in ("ForStmt", "WhileStmt", "DoStmt", "IfStmt", "SwitchStmt", "ReturnStmt"):
                ast_branch = ast_branch.add(f"[yellow]{node_name}[/]")
            elif "Decl" in node_name:
                ast_branch = ast_branch.add(f"[magenta]{node_name}[/]")
            elif "Expr" in node_name or "Operator" in node_name:
                ast_branch = ast_branch.add(f"[green]{node_name}[/]")
            else:
                ast_branch = ast_branch.add(f"[white]{node_name}[/]")

        ctx_branch = tree.add("[bold]Detected Contexts[/]")
        for cl in context_lines:
            ctx_branch.add(f"[bold green]{cl}[/]")

        if not context_lines:
            ctx_branch.add("[dim]ASSIGNMENT_CONTEXT (default)[/dim]")

        console.print(tree)
        console.print()

# ─── /report — Raw Full Report ────────────────────────────────────────────────

def handle_report(args):
    if not args:
        console.print("[red]Usage: /report <file.c>[/]")
        return

    file_path = args[0]
    if not os.path.exists(file_path):
        console.print(f"[bold red]Error: File '{file_path}' not found.[/]")
        return

    console.print(Rule(f"[bold cyan]Full CLI Report: {file_path}[/]"))

    cmd = ["./build/src/icha", file_path, "--"]
    with Progress(SpinnerColumn("dots"), TextColumn("[cyan]Generating report...[/]"), console=console, transient=True) as p:
        t = p.add_task("Running...", total=None)
        result = subprocess.run(cmd, capture_output=True, text=True)
        p.update(t, completed=True)

    console.print(result.stdout)

# ─── /benchmark — Run OSS Evaluation Suite ────────────────────────────────────

def handle_benchmark():
    console.print(Rule("[bold cyan]Running Implicit Conversion Hazard Analyzer OSS Evaluation & Benchmarking[/]"))
    console.print("[dim]Analyzing SQLite, validation suite and sample projects. Please wait...[/dim]\n")
    
    cmd = [
        sys.executable,
        "benchmarking/src/run_benchmark.py",
        "--config", "benchmarking/config/projects.json",
        "--icha-binary", "./build/src/icha",
        "--output-dir", "benchmarking/results"
    ]
    
    with Progress(
        SpinnerColumn("dots"),
        TextColumn("[cyan]Running Benchmarks & CVE Correlation...[/]"),
        console=console,
        transient=True
    ) as p:
        t = p.add_task("Running...", total=None)
        result = subprocess.run(cmd, capture_output=True, text=True)
        p.update(t, completed=True)
        
    if result.returncode != 0:
        console.print("[bold red]Benchmark Failed![/]")
        console.print(result.stderr)
        return
        
    console.print("[bold green]✓ Benchmarks Completed Successfully![/bold green]\n")
    
    stats_file = "benchmarking/results/statistical_summary.txt"
    if os.path.exists(stats_file):
        with open(stats_file, 'r') as f:
            stats = f.read()
        console.print(Panel(stats, title="[bold green]Statistical Summary[/bold green]", border_style="green", box=box.ROUNDED))
    else:
        console.print("[red]Error: statistical_summary.txt not generated.[/]")

# ─── /compare — Side-by-Side Tool Comparison ──────────────────────────────────

def handle_compare():
    console.print(Rule("[bold cyan]Implicit Conversion Hazard Analyzer vs Clang Baseline Comparison[/]"))
    
    comp_file = "benchmarking/results/comparison_table.txt"
    if os.path.exists(comp_file):
        with open(comp_file, 'r') as f:
            comp_data = f.read()
        console.print(Panel(comp_data, title="[bold green]Side-by-Side Comparison[/bold green]", border_style="green", box=box.ROUNDED))
    else:
        console.print("[yellow]No benchmark results found. Run [bold cyan]/benchmark[/bold cyan] first to generate comparison data.[/yellow]")

# ─── /html — Interactive HTML Dashboard ───────────────────────────────────────

def handle_html(args):
    if not args:
        console.print("[red]Usage: /html <file.c>[/]")
        return

    file_path = args[0]
    if not os.path.exists(file_path):
        console.print(f"[bold red]Error: File '{file_path}' not found.[/]")
        return

    output_html = "report.html"
    temp_json = "temp_report.json"
    
    console.print(Rule(f"[bold cyan]Generating Premium HTML Report: {file_path}[/]"))
    
    # Run ICHA with JSON format output first
    cmd = ["./build/src/icha", "-icha-format=json", f"-icha-output={temp_json}", file_path, "--"]
    
    # Append macOS sysroot SDK/header paths to compile successfully
    cmd.extend([
        "-isysroot", "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk",
        "-I/Library/Developer/CommandLineTools/usr/lib/clang/15.0.0/include"
    ])
    
    with Progress(SpinnerColumn("dots"), TextColumn("[cyan]Analyzing code & generating JSON...[/]"), console=console, transient=True) as p:
        t = p.add_task("Running...", total=None)
        result = subprocess.run(cmd, capture_output=True, text=True)
        p.update(t, completed=True)

    if not os.path.exists(temp_json):
        console.print("[bold red]Error: Failed to generate temporary JSON report.[/]")
        if result.stderr:
            console.print(result.stderr)
        return

    # Use html_reporter to compile into HTML dashboard
    try:
        from html_reporter import generate_html_report
        generate_html_report(temp_json, output_html)
        
        # Clean up temp json
        if os.path.exists(temp_json):
            os.remove(temp_json)
        
        console.print(f"[bold green]✓ Dashboard successfully created at [underline]{output_html}[/underline]![/bold green]")
        console.print("[dim]Opening in your default browser...[/dim]")
        
        # Open in default browser
        import webbrowser
        webbrowser.open('file://' + os.path.realpath(output_html))
    except Exception as e:
        console.print(f"[bold red]Failed to compile HTML dashboard:[/] {e}")

# ─── Main Loop ────────────────────────────────────────────────────────────────

def main():
    print_welcome()
    session = PromptSession(completer=completer, style=prompt_style)

    while True:
        try:
            text = session.prompt('icha❯ ')
            text = text.strip()
            if not text:
                continue

            parts = text.split()
            cmd = parts[0].lower()
            args = parts[1:]

            if cmd in ('/exit', '/quit'):
                console.print("[cyan]👋 Goodbye![/]")
                break
            elif cmd == '/help':      print_help()
            elif cmd == '/clear':     os.system('clear' if os.name == 'posix' else 'cls')
            elif cmd == '/build':     handle_build()
            elif cmd == '/test':      handle_test()
            elif cmd == '/status':    handle_status()
            elif cmd == '/analyze':   handle_analyze(args)
            elif cmd == '/inspect':   handle_inspect(args)
            elif cmd == '/debug':     handle_debug(args)
            elif cmd == '/report':    handle_report(args)
            elif cmd == '/benchmark': handle_benchmark()
            elif cmd == '/compare':   handle_compare()
            elif cmd == '/html':      handle_html(args)
            elif cmd.startswith('/'):
                console.print(f"[red]Unknown command:[/] {cmd}. Type [cyan]/help[/] for commands.")
            elif os.path.exists(text):
                handle_analyze([text])
            else:
                console.print(f"[red]Unknown command or file not found:[/] {text}. Type [cyan]/help[/].")

        except KeyboardInterrupt:
            continue
        except EOFError:
            console.print("\n[cyan]👋 Goodbye![/]")
            break
        except OSError:
            console.print("[red]Terminal disconnected. Exiting...[/]")
            break
        except Exception as e:
            console.print(f"[bold red]Error:[/] {str(e)}")

if __name__ == '__main__':
    main()
