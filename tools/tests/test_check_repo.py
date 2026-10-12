"""Behavioral tests for content and link validation, using synthetic data only."""

import importlib.util
import subprocess
import tempfile
import unittest
from pathlib import Path

MODULE = Path(__file__).resolve().parents[1] / "check_repo.py"
SPEC = importlib.util.spec_from_file_location("check_repo", MODULE)
checker = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(checker)


class ContentTests(unittest.TestCase):
    def test_public_project_and_placeholder_email_are_allowed(self):
        text = "https://github.com/AfshinKI/openpaint\ncontact@example.org\n"
        self.assertEqual(checker.content_problems(text), [])

    def test_private_patterns_report_labels_without_values(self):
        token = "gh" + "p_" + "a" * 36
        home = "/" + "home" + "/private-user/project"
        email = "private-person" + "@" + "private-mail.test"
        key = "-----BEGIN " + "PRIVATE KEY-----"
        findings = checker.content_problems("\n".join((token, home, email, key)) + "\n")
        labels = [label for _, label in findings]
        self.assertIn("GitHub token", labels)
        self.assertIn("personal home path", labels)
        self.assertIn("email literal: review privacy/provenance", labels)
        self.assertIn("private key", labels)
        self.assertNotIn(token, str(findings))
        self.assertNotIn(email, str(findings))

    def test_format_errors(self):
        findings = checker.content_problems("hello  \r\nlast")
        self.assertEqual({label for _, label in findings},
                         {"trailing whitespace", "non-LF line ending", "missing final newline"})

    def test_private_filenames(self):
        for name in (".env", ".env.production", "signing.key", "id_ed25519", "CLAUDE.local.md"):
            self.assertTrue(checker.private_filename(Path(name)), name)
        self.assertFalse(checker.private_filename(Path(".env.example")))


class LinkTests(unittest.TestCase):
    def test_relative_links_fragments_and_external_urls(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            source = root / "README.md"
            target = root / "Guide.md"
            target.write_text("# Good heading\n\n## Good heading\n", encoding="utf-8")
            content = "[local](Guide.md#good-heading-1) [web](https://example.org/missing)\n"
            self.assertEqual(checker.link_problems(root, source, content), [])

    def test_broken_link_fragment_and_escape_fail(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            source = root / "README.md"
            source.write_text("# Title\n", encoding="utf-8")
            content = "[missing](absent.md) [anchor](#absent) [escape](../secret.md)\n"
            labels = {label for _, label in checker.link_problems(root, source, content)}
            self.assertEqual(labels, {"missing local link target", "missing local heading anchor",
                                      "documentation link escapes repository"})

    def test_tracked_ignored_files_still_scanned(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory).resolve()
            subprocess.run(["git", "init", "-q", str(root)], check=True)
            (root / ".gitignore").write_text(".env*\n", encoding="utf-8")
            tracked = root / ".env.tracked"
            tracked.write_text("synthetic fixture\n", encoding="utf-8")
            (root / ".env.untracked").write_text("synthetic ignored fixture\n", encoding="utf-8")
            subprocess.run(["git", "add", "-f", ".env.tracked"], cwd=root, check=True)
            files = checker.candidate_files(root)
            self.assertIn(tracked, files)
            self.assertNotIn(root / ".env.untracked", files)


if __name__ == "__main__":
    unittest.main()
