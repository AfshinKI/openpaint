"""DEB version ordering and fail-closed dependency handling."""
import importlib.util
from pathlib import Path
import subprocess
import shutil
import tempfile
import unittest

SPEC = importlib.util.spec_from_file_location('package_deb', Path(__file__).parents[1] / 'package_deb.py')
PACKAGE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(PACKAGE)


class DebianPackageTests(unittest.TestCase):
    def test_prerelease_upgrades_to_stable(self):
        self.assertEqual(PACKAGE.deb_version('1.0.0'), '1.0.0-1')
        before = PACKAGE.deb_version('1.0.0-alpha.12')
        after = PACKAGE.deb_version('1.0.0')
        subprocess.run(['dpkg', '--compare-versions', before, 'lt', after], check=True)

    def test_invalid_versions_rejected(self):
        for value in ['v1.0.0', '1.0', '1.0.0\nDepends: anything', '../1.0.0', '01.0.0']:
            with self.subTest(value=value), self.assertRaises(ValueError):
                PACKAGE.deb_version(value)

    def test_empty_runtime_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            with self.assertRaisesRegex(ValueError, 'No ELF'):
                PACKAGE.dependencies(Path(directory), Path(directory))

    def test_elf_scan_ignores_links_and_text(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'elf').write_bytes(b'\x7fELFfixture')
            (root / 'text').write_text('ordinary file')
            (root / 'alias').symlink_to('elf')
            self.assertEqual(PACKAGE.elf_files(root), [root / 'elf'])

    @unittest.skipUnless(shutil.which('cc') and shutil.which('dpkg-shlibdeps'), 'C compiler and dpkg-dev required')
    def test_unshipped_library_fails_dependency_generation(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            stage = root / 'stage'
            stage.mkdir()
            library = root / 'libmissing.so.1'
            source = root / 'library.c'
            source.write_text('int dependency(void) { return 0; }')
            subprocess.run(['cc', '-shared', '-fPIC', '-Wl,-soname,libmissing.so.1', str(source), '-o', str(library)], check=True)
            source.write_text('extern int dependency(void); int main(void) { return dependency(); }')
            subprocess.run(['cc', str(source), str(library), '-o', str(stage / 'app')], check=True)
            library.unlink()
            with self.assertRaises(subprocess.CalledProcessError):
                PACKAGE.dependencies(stage, root)
