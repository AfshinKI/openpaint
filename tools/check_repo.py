#!/usr/bin/env python3
"""Offline checks for publishable repository content; not a full secret scanner."""

from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path
from urllib.parse import unquote, urlsplit

ROOT = Path(__file__).resolve().parents[1]
SKIP_DIRS = {
    ".git", ".codex", ".agents", ".claude", ".venv", "__pycache__",
    "build", "out", "dist", "artifacts", "private", "scratch", "tmp", "logs",
    ".idea", ".vscode", ".pytest_cache",
}
TEXT_SUFFIXES = {".md", ".py", ".yml", ".yaml", ".json", ".toml", ".txt",
                 ".cmake", ".cpp", ".h", ".hpp", ".qml", ".svg", ".html",
                 ".sh", ".xml", ".desktop", ".in"}
TEXT_NAMES = {"AppRun", "LICENSE", "CMakeLists.txt", ".gitignore", ".gitattributes", ".editorconfig"}
REQUIRED = ("README.md", "AGENTS.md", "CLAUDE.md", "CONTRIBUTING.md", "LICENSE",
            "SECURITY.md", "docs/README.md", "docs/ROADMAP.md", "docs/FEATURES.md",
            "docs/ARCHITECTURE.md", "docs/DESIGN.md", "docs/DEVELOPMENT.md",
            "docs/QUALITY.md", "docs/RELEASES.md", "docs/PRIVACY.md")
PATTERNS = (
    ("private key", re.compile(r"-----BEGIN (?:RSA |EC |OPENSSH |DSA )?PRIVATE KEY-----")),
    ("GitHub token", re.compile(r"\b(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,})\b")),
    ("API key", re.compile(r"\bsk-(?:proj-|ant-)?[A-Za-z0-9_-]{24,}\b")),
    ("cloud access key", re.compile(r"\b(?:AKIA|ASIA)[A-Z0-9]{16}\b")),
    ("personal home path", re.compile(r"(?:/(?:home|Users)/[A-Za-z0-9_.-]+|[A-Za-z]:\\Users\\[A-Za-z0-9_.-]+)")),
)
EMAIL = re.compile(r"\b[A-Za-z0-9._%+-]+@([A-Za-z0-9.-]+\.[A-Za-z]{2,})\b")
LINK = re.compile(r"!?\[[^\]\n]*\]\(([^\s)]+)(?:\s+\"[^\"]*\")?\)")


def candidate_files(root: Path) -> list[Path]:
    """Include tracked files even if ignored, plus nonignored new files."""
    try:
        top = subprocess.run(
            ["git", "rev-parse", "--show-toplevel"], cwd=root,
            capture_output=True, text=True, check=True,
        )
        if Path(top.stdout.strip()).resolve() != root.resolve():
            raise ValueError("not the repository root")
        result = subprocess.run(
            ["git", "ls-files", "--cached", "--others", "--exclude-standard", "-z"],
            cwd=root, capture_output=True, check=True,
        )
        return sorted({root / name.decode("utf-8") for name in result.stdout.split(b"\0") if name})
    except (FileNotFoundError, subprocess.CalledProcessError, ValueError):
        # The initial documentation tree may not have been initialized with Git yet.
        return sorted(path for path in root.rglob("*")
                      if not any(part in SKIP_DIRS for part in path.relative_to(root).parts)
                      and (path.is_file() or path.is_symlink()))


def private_filename(path: Path) -> bool:
    name = path.name
    return (
        (name.startswith(".env") and name != ".env.example")
        or path.suffix.lower() in {".pem", ".key", ".p12", ".pfx", ".dmp"}
        or name in {"id_rsa", "id_ed25519", "CLAUDE.local.md", "AGENTS.local.md"}
        or any(part in {".codex", ".agents", ".claude", "private"} for part in path.parts)
    )


def content_problems(content: str) -> list[tuple[int, str]]:
    """Return only rule names and line numbers, never matched secret values."""
    problems = []
    for number, line in enumerate(content.splitlines(), 1):
        if line.rstrip() != line:
            problems.append((number, "trailing whitespace"))
        for label, pattern in PATTERNS:
            if pattern.search(line):
                problems.append((number, label))
        for match in EMAIL.finditer(line):
            domain = match.group(1).lower()
            if domain not in {"example.com", "example.org", "example.net",
                              "users.noreply.github.com"}:
                problems.append((number, "email literal: review privacy/provenance"))
    if content and not content.endswith("\n"):
        problems.append((len(content.splitlines()), "missing final newline"))
    if "\r" in content:
        problems.append((1, "non-LF line ending"))
    return problems


def headings(content: str) -> set[str]:
    """GitHub-style slugs for ordinary Markdown headings used in these docs."""
    anchors, counts = set(), {}
    for line in content.splitlines():
        match = re.match(r"^#{1,6}\s+(.+?)\s*#*\s*$", line)
        if not match:
            continue
        slug = re.sub(r"[^\w\- ]", "", match.group(1).lower()).replace(" ", "-")
        count = counts.get(slug, 0)
        counts[slug] = count + 1
        anchors.add(f"{slug}-{count}" if count else slug)
    return anchors


def link_problems(root: Path, source: Path, content: str) -> list[tuple[int, str]]:
    problems = []
    for match in LINK.finditer(content):
        destination = urlsplit(match.group(1).strip("<>"))
        if destination.scheme in {"http", "https", "mailto"}:
            continue  # This is deliberately an offline check.
        number = content.count("\n", 0, match.start()) + 1
        if destination.scheme or destination.netloc or destination.path.startswith("/"):
            problems.append((number, "nonportable documentation link"))
            continue
        target = (source.parent / unquote(destination.path)).resolve() if destination.path else source
        if not target.is_relative_to(root.resolve()):
            problems.append((number, "documentation link escapes repository"))
        elif not target.exists():
            problems.append((number, "missing local link target"))
        elif destination.fragment and target.suffix == ".md":
            if unquote(destination.fragment) not in headings(target.read_text(encoding="utf-8")):
                problems.append((number, "missing local heading anchor"))
    return problems


def check(root: Path) -> tuple[list[str], int]:
    errors, count = [], 0
    for name in REQUIRED:
        if not (root / name).is_file():
            errors.append(f"{name}: required file missing")
    claude = root / "CLAUDE.md"
    if claude.is_file() and "@AGENTS.md" not in claude.read_text(encoding="utf-8"):
        errors.append("CLAUDE.md: must import the shared AGENTS.md instructions")
    for path in candidate_files(root):
        relative = path.relative_to(root)
        if path.is_symlink():
            errors.append(f"{relative}: symlink requires an explicit reviewed policy")
            continue
        if not path.is_file():
            continue  # A tracked file may have been deleted in the working tree.
        count += 1
        if private_filename(relative):
            errors.append(f"{relative}: private/local filename must not be published")
        if path.suffix.lower() not in TEXT_SUFFIXES and path.name not in TEXT_NAMES:
            continue  # Binary/media content needs a separate manual review.
        try:
            content = path.read_bytes().decode("utf-8")
        except UnicodeDecodeError:
            errors.append(f"{relative}: expected UTF-8 text")
            continue
        # Preserve the upstream license verbatim; its attribution is not private data.
        if path.name == "LICENSE":
            continue
        problems = content_problems(content)
        if path.suffix == ".md":
            problems += link_problems(root, path, content)
        errors.extend(f"{relative}:{line}: {label}" for line, label in problems)
    return errors, count


def main() -> int:
    errors, count = check(ROOT)
    if errors:
        print("Repository checks failed:")
        for error in errors:
            print(f"  {error}")
        return 1
    print(f"Repository checks passed ({count} files; local links, text hygiene, selected privacy patterns).")
    print("External URLs, media contents, Git history, and commit identities require separate review.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
