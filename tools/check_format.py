#!/usr/bin/env python3
"""Compare formatter output without editing source; tool failures fail the check."""

import argparse
import subprocess
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--clang-format", required=True)
    parser.add_argument("--qmlformat", required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    paths = sorted(path for directory in ("src", "tests", "qml")
                   for path in (root / directory).rglob("*")
                   if path.suffix in {".cpp", ".h", ".hpp", ".qml"})
    failed = False
    for path in paths:
        tool = args.qmlformat if path.suffix == ".qml" else args.clang_format
        result = subprocess.run([tool, str(path)], capture_output=True, check=True)
        if result.stdout != path.read_bytes():
            print(f"Formatting required: {path.relative_to(root)}")
            failed = True
    if not failed:
        print(f"Formatting passed ({len(paths)} C++/QML files).")
    return int(failed)


if __name__ == "__main__":
    raise SystemExit(main())
