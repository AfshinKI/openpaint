"""Release website checks must reject stale release metadata."""
import json
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


class WebsiteReleaseTests(unittest.TestCase):
    def test_current_release_builds_and_stale_release_fails(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for name in ('website', 'qml/icons', 'docs/assets'):
                shutil.copytree(ROOT / name, root / name)
            (root / 'tools').mkdir()
            for name in ('tools/build_site.py', 'CMakeLists.txt', 'LICENSE'):
                shutil.copyfile(ROOT / name, root / name)
            command = [sys.executable, str(root / 'tools/build_site.py')]
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertTrue((root / 'build/website/index.html').is_file())
            metadata = root / 'website/release.json'
            release = json.loads(metadata.read_text())
            release['version'] = '0.0.0'
            metadata.write_text(json.dumps(release))
            result = subprocess.run(command, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn('Review website screenshots', result.stderr)
