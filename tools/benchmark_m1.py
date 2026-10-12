#!/usr/bin/env python3
"""Run the opt-in M1 probe against a Release build on a chosen Qt display backend."""
import os
from pathlib import Path
import re
import statistics
import subprocess

root = Path(__file__).resolve().parent.parent
binary = root / "build/release/openpaint_shell_tests"
startup = []
for iteration in range(35):
    result = subprocess.run([str(binary), "launchAndResize"], text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            env=dict(os.environ, OPENPAINT_BENCHMARK_STARTUP="1"), timeout=30)
    if result.returncode:
        raise SystemExit("Startup probe failed; run the shell test directly for diagnostics.")
    match = re.search(r"M1 startup_ms ([0-9.]+)", result.stdout)
    if match is None:
        raise SystemExit("Startup measurement was not reported.")
    if iteration >= 5:
        startup.append(float(match.group(1)))
print(f"M1 startup p50_ms {statistics.median(startup):.3f} "
      f"p95_ms {sorted(startup)[28]:.3f} samples {len(startup)}", flush=True)
result = subprocess.run([str(binary), "renderTiming"], text=True,
                        stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                        env=dict(os.environ, OPENPAINT_BENCHMARK="1"), timeout=90)
for line in result.stdout.splitlines():
    if "M1 " in line:
        print(line[line.index("M1 "):])
if result.returncode:
    raise SystemExit("Render/resource probe failed; run the shell test directly for diagnostics.")
