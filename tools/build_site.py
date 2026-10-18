#!/usr/bin/env python3
"""Stage the dependency-free project website for GitHub Pages."""
from pathlib import Path
import shutil
import json
import re

ROOT = Path(__file__).resolve().parents[1]
release = json.loads((ROOT / "website" / "release.json").read_text())
version = re.search(r"project\(OpenPaint VERSION ([0-9.]+)", (ROOT / "CMakeLists.txt").read_text()).group(1)
if release["version"] != version:
    raise SystemExit("Review website screenshots and release.json for the new application version")
OUT = ROOT / "build" / "website"
OUT.mkdir(parents=True, exist_ok=True)
for name in ("index.html", "style.css"):
    shutil.copyfile(ROOT / "website" / name, OUT / name)
assets = OUT / "assets"
assets.mkdir(exist_ok=True)
for name in ("app", "brush", "palette", "crop", "text", "selection", "undo"):
    shutil.copyfile(ROOT / "qml" / "icons" / f"{name}.svg", assets / f"{name}.svg")
for name in release["screenshots"]:
    shutil.copyfile(ROOT / "docs" / "assets" / name, assets / name)
shutil.copyfile(ROOT / "LICENSE", OUT / "LICENSE")
(OUT / ".nojekyll").touch()
print("Website staged in build/website")
