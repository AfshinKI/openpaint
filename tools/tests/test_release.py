import io
import json
from pathlib import Path
import sys
import tarfile
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from release import digest, identity, stage


class ReleaseTests(unittest.TestCase):
    def test_stable_requires_explicit_main_dispatch_and_matching_version(self):
        self.assertEqual(identity('1.0.0', '1.0.0', 'workflow_dispatch', 'refs/heads/main', '42'),
                         {'version': '1.0.0', 'suffix': '', 'publish': 'true'})
        for event, ref, version in [('push', 'refs/heads/main', '1.0.0'),
                                    ('workflow_dispatch', 'refs/heads/topic', '1.0.0'),
                                    ('workflow_dispatch', 'refs/heads/main', '1.1.0'),
                                    ('workflow_dispatch', 'refs/heads/main', 'v1.0.0')]:
            with self.assertRaises(ValueError):
                identity(version, '1.0.0', event, ref, '42')
        self.assertEqual(identity('', '1.0.0', 'push', 'refs/heads/main', '42')['publish'], 'false')

    def fixture(self, root):
        commit = 'a' * 40
        for kind, binary in [('portable', 'OpenPaint-1.0.0-linux-x86_64.tar.gz'),
                             ('deb', 'OpenPaint-1.0.0-ubuntu24.04-amd64.deb'),
                             ('flatpak', 'OpenPaint-1.0.0-x86_64.flatpak')]:
            directory = root / kind
            directory.mkdir()
            (directory / binary).write_bytes(b'synthetic package')
            inventory = {'version': '1.0.0', 'source_commit': commit}
            if kind == 'flatpak':
                (directory / 'openpaint-source.tar').write_bytes(b'synthetic source')
                inventory.update(bundle_sha256=digest(directory / binary),
                                 source_sha256=digest(directory / 'openpaint-source.tar'))
                (directory / 'flatpak-inventory.json').write_text(json.dumps(inventory))
                (directory / 'manifest.json').write_text('{}')
            else:
                with tarfile.open(directory / 'OpenPaint-1.0.0-sources-and-notices.tar', 'w') as archive:
                    data = json.dumps(inventory).encode()
                    info = tarfile.TarInfo('dependency-inventory.json')
                    info.size = len(data)
                    archive.addfile(info, io.BytesIO(data))
            paths = list(directory.iterdir())
            (directory / 'SHA256SUMS').write_text(''.join(f'{digest(p)}  {p.name}\n' for p in paths))
        return commit

    def test_complete_release_retains_all_compliance_material(self):
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            commit = self.fixture(root)
            stage(root, root / 'release', '1.0.0', commit)
            self.assertEqual(len(list((root / 'release').iterdir())), 5)
            with tarfile.open(root / 'release/OpenPaint-1.0.0-sources-and-notices.tar') as archive:
                self.assertEqual(len(archive.getnames()), 5)

    def test_tampered_missing_wrong_version_and_wrong_commit_fail_before_staging(self):
        for failure in ('tampered', 'missing', 'version', 'commit', 'checksum', 'traversal'):
            with self.subTest(failure=failure), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                commit = self.fixture(root)
                path = root / 'deb/OpenPaint-1.0.0-ubuntu24.04-amd64.deb'
                version = '1.0.0'
                if failure == 'tampered':
                    path.write_bytes(b'changed')
                elif failure == 'missing':
                    path.unlink()
                elif failure == 'version':
                    version = '1.1.0'
                elif failure == 'commit':
                    commit = 'b' * 40
                elif failure == 'checksum':
                    (root / 'deb/SHA256SUMS').write_text('')
                else:
                    (root / 'deb/SHA256SUMS').write_text('a' * 64 + '  ../outside\n')
                with self.assertRaises((ValueError, FileNotFoundError)):
                    stage(root, root / 'release', version, commit)
                self.assertFalse((root / 'release').exists())


class PublicationTests(unittest.TestCase):
    def test_published_is_untouched_and_failed_upload_stays_draft(self):
        import os
        import subprocess
        script = Path(__file__).resolve().parents[1] / 'publish_release.sh'
        for mode in ('published', 'upload-failure', 'success', 'wrong-target'):
            with self.subTest(mode=mode), tempfile.TemporaryDirectory() as temporary:
                root = Path(temporary)
                (root / 'packaging').mkdir()
                (root / 'packaging/release-notes.md').write_text('Version {VERSION}')
                (root / 'release').mkdir()
                (root / 'release/SHA256SUMS').write_text('synthetic')
                bin_dir = root / 'bin'
                bin_dir.mkdir()
                fake = '''#!/usr/bin/env python3
import json, os, sys
from pathlib import Path
args = sys.argv[1:]
name = Path(sys.argv[0]).name
with open('calls', 'a') as log:
    log.write(name + ' ' + ' '.join(args) + '\\n')
mode = os.environ['TEST_MODE']
if name == 'git':
    sys.exit(2)  # No pre-existing tag.
if name == 'gh' and args[:2] == ['release', 'view']:
    print(json.dumps({'isDraft': mode != 'published', 'targetCommitish': ('b' if mode == 'wrong-target' else 'a') * 40}))
if name == 'gh' and args[:2] == ['release', 'upload'] and mode == 'upload-failure':
    sys.exit(1)
'''
                for name in ('gh', 'git', 'sha256sum'):
                    path = bin_dir / name
                    path.write_text(fake)
                    path.chmod(0o755)
                result = subprocess.run(['bash', str(script)], cwd=root, capture_output=True,
                                        env=dict(os.environ, PATH=str(bin_dir) + ':' + os.environ['PATH'],
                                                 RELEASE_VERSION='1.0.0', SOURCE_SHA='a' * 40,
                                                 GH_REPO='example/project', TEST_MODE=mode))
                calls = (root / 'calls').read_text()
                self.assertEqual('release edit' in calls, mode == 'success')
                self.assertEqual('release upload' in calls, mode in ('success', 'upload-failure'))
                self.assertEqual(result.returncode == 0, mode in ('published', 'success'))


if __name__ == '__main__':
    unittest.main()
