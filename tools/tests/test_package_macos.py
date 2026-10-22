import importlib.util
import io
import tarfile
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
    def test_only_self_contained_plugin_types_are_staged(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            qt, bundle = root / 'qt', root / 'OpenPaint.app'
            included = ['platforms/libqcocoa.dylib', 'imageformats/libqwebp.dylib',
                        'tls/libqsecuretransportbackend.dylib', 'sqldrivers/libqsqlite.dylib']
            excluded = ['platforms/libqoffscreen.dylib', 'imageformats/libqwebp_debug.dylib',
                        'sqldrivers/libqsqlpsql.dylib', 'sqldrivers/libqsqlmimer.dylib']
            for name in included + excluded:
                path = qt / 'plugins' / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b'generated plugin fixture')
            deployed = package.stage_plugins(qt, bundle)
            self.assertEqual({str(p.relative_to(bundle / 'Contents/PlugIns')) for p in deployed},
                             set(included))
            self.assertTrue(all((qt / 'plugins' / name).is_file() for name in excluded))

    def test_source_collection_creates_cache_and_retains_notices(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            archive = root / 'generated-source.tar'
            notice = b'Generated fixture license notice'
            with tarfile.open(archive, 'w') as output:
                member = tarfile.TarInfo('fixture/LICENSE')
                member.size = len(notice)
                output.addfile(member, io.BytesIO(notice))
            entry = dict(name=archive.name, url=archive.as_uri(), sha256=package.digest(archive))
            cache, sources, notices = (root / name for name in ('cache', 'sources', 'notices'))
            package.collect_sources([entry], cache, sources, notices)
            self.assertEqual(package.digest(sources / archive.name), entry['sha256'])
            self.assertEqual((notices / archive.name / 'fixture/LICENSE').read_bytes(), notice)
            self.assertFalse(list(cache.glob('*.partial')))

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

    def test_universal_architecture_headings_are_not_dependencies(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            binary = root / 'Contents/PlugIns/platforms/libqcocoa.dylib'
            binary.parent.mkdir(parents=True)
            binary.write_bytes(bytes.fromhex('cafebabe') + b'generated universal binary')
            output = ''.join(f'{binary} (architecture {arch}):\n'
                             '\t@rpath/QtCore.framework/Versions/A/QtCore (compatibility version 6.0.0)\n'
                             '\t/usr/lib/libSystem.B.dylib (compatibility version 1.0.0)\n'
                             for arch in ('arm64', 'x86_64'))
            with patch.object(subprocess, 'check_output', return_value=output):
                self.assertEqual(len(package.audit_bundle(root)), 1)

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
