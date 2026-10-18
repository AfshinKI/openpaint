#!/usr/bin/env python3
"""Stage the dependency-free project website for GitHub Pages."""
from pathlib import Path
import shutil

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "build" / "website"
OUT.mkdir(parents=True, exist_ok=True)
for name in ("index.html", "style.css"):
    shutil.copyfile(ROOT / "website" / name, OUT / name)
assets = OUT / "assets"
assets.mkdir(exist_ok=True)
for name in ("app", "brush", "palette", "crop", "text", "selection", "undo"):
    shutil.copyfile(ROOT / "qml" / "icons" / f"{name}.svg", assets / f"{name}.svg")
for name in ("openpaint-frost-light.png", "color-editor-dark-1024.png", "resize-light-1024.png"):
    shutil.copyfile(ROOT / "docs" / "assets" / name, assets / name)
shutil.copyfile(ROOT / "LICENSE", OUT / "LICENSE")
(OUT / ".nojekyll").touch()
print("Website staged in build/website")
