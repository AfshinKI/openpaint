"""Behavioral checks for package integrity and archive handling."""
import hashlib
import importlib.util
import io
from pathlib import Path
import tarfile
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location('package_appimage', Path(__file__).parents[1] / 'package_appimage.py')
PACKAGE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PACKAGE)


class PackageTests(unittest.TestCase):
    def test_portable_archive_preserves_launcher_and_bundled_libraries(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            appdir = root / 'AppDir'
            (appdir / 'usr/lib').mkdir(parents=True)
            (appdir / 'AppRun').write_text('#!/bin/sh\nexit 0\n')
            (appdir / 'usr/lib/library.so').write_bytes(b'library')
            archive = PACKAGE.portable_archive(appdir, root, '0.1.0-alpha.10')
            with tarfile.open(archive) as bundle:
                prefix = 'OpenPaint-0.1.0-alpha.10-linux-x86_64'
                launcher = bundle.getmember(prefix + '/openpaint')
                self.assertEqual(launcher.mode, 0o755)
                self.assertEqual((launcher.uid, launcher.gid), (0, 0))
                self.assertEqual(bundle.extractfile(launcher).read(), b'#!/bin/sh\nexit 0\n')
                self.assertEqual(bundle.extractfile(prefix + '/usr/lib/library.so').read(), b'library')

    def test_source_tar_omits_local_owner(self):
        info = tarfile.TarInfo('source.tar.xz')
        info.uname = 'local-builder'
        info.gname = 'local-group'
        info.uid = info.gid = 1000
        info.mtime = 123456789
        clean = PACKAGE.public_tar_info(info)
        self.assertEqual((clean.uname, clean.gname, clean.uid, clean.gid, clean.mtime), ('', '', 0, 0, 0))

    def test_bad_cache_is_not_executed(self):
        with tempfile.TemporaryDirectory() as directory:
            cache = Path(directory)
            (cache / 'tool').write_bytes(b'changed')
            entry = {'name': 'tool', 'url': 'https://example.invalid/tool', 'sha256': hashlib.sha256(b'original').hexdigest()}
            with self.assertRaisesRegex(ValueError, 'Cached digest mismatch'):
                PACKAGE.fetch(entry, cache)

    def test_license_extraction_does_not_follow_links(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            archive = root / 'source.tar'
            with tarfile.open(archive, 'w') as output:
                member = tarfile.TarInfo('source/LICENSE')
                member.size = 7
                output.addfile(member, io.BytesIO(b'license'))
                link = tarfile.TarInfo('source/COPYING')
                link.type = tarfile.SYMTYPE
                link.linkname = '/etc/passwd'
                output.addfile(link)
            PACKAGE.license_files(archive, root / 'licenses')
            self.assertEqual((root / 'licenses/source/LICENSE').read_text(), 'license')
            self.assertFalse((root / 'licenses/source/COPYING').exists())

    def test_archive_traversal_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            archive = root / 'source.tar'
            with tarfile.open(archive, 'w') as output:
                output.addfile(tarfile.TarInfo('../LICENSE'))
            with self.assertRaisesRegex(ValueError, 'Unsafe source archive'):
                PACKAGE.license_files(archive, root / 'licenses')

    def test_artifact_paths_and_required_plugins(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'binary').write_bytes(b'private-build-prefix')
            with self.assertRaisesRegex(ValueError, 'Local build path'):
                PACKAGE.audit(root, [b'private-build-prefix'])
            (root / 'binary').write_bytes(b'clean')
            with self.assertRaisesRegex(ValueError, 'Missing runtime component'):
                PACKAGE.audit(root, [])
            (root / 'escape').symlink_to(root.parent)
            with self.assertRaisesRegex(ValueError, 'symlink escapes'):
                PACKAGE.audit(root, [])
