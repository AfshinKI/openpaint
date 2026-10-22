import importlib.util
from pathlib import Path
import subprocess
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import tempfile
import unittest
from unittest.mock import patch

spec = importlib.util.spec_from_file_location("package_macos", Path(__file__).resolve().parents[1] / "package_macos.py")
package = importlib.util.module_from_spec(spec)
spec.loader.exec_module(package)


class MacPackageTests(unittest.TestCase):
    def test_required_plugin_and_private_paths(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            with self.assertRaisesRegex(ValueError, "Missing Cocoa"):
                package.audit_bundle(root)
            plugin = root / "Contents/PlugIns/platforms/libqcocoa.dylib"
            plugin.parent.mkdir(parents=True)
            plugin.write_bytes(b"synthetic plugin")
            self.assertEqual(len(package.audit_bundle(root)), 1)
            plugin.write_bytes(str(package.ROOT).encode())
            with self.assertRaisesRegex(ValueError, "Build path leaked"):
                package.audit_bundle(root)

    def test_external_dependency_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            binary = root / "binary"
            binary.write_bytes(bytes.fromhex("cffaedfe") + b"generated")
            with patch.object(subprocess, "check_output", return_value="binary:\n\t/opt/unbundled/library.dylib (version 1)\n"):
                with self.assertRaisesRegex(ValueError, "External runtime"):
                    package.audit_bundle(root)

    def test_escaping_link_is_rejected(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            (root / "escape").symlink_to(root.parent)
            with self.assertRaisesRegex(ValueError, "symlink escapes"):
                package.audit_bundle(root)
