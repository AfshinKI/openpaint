#!/usr/bin/env python3
"""Build a reviewed Flatpak bundle without publishing or changing host installs."""
import argparse
import configparser
import hashlib
import json
from pathlib import Path
import re
import subprocess

ROOT = Path(__file__).resolve().parents[1]
APP_ID = 'io.github.AfshinKI.OpenPaint'


def run(*args, **kwargs):
    return subprocess.run(args, check=True, text=True, **kwargs)


def output(*args):
    return run(*args, capture_output=True).stdout.strip()


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def audit_permissions(text):
    metadata = configparser.ConfigParser(interpolation=None)
    metadata.optionxform = str
    metadata.read_string(text)
    allowed = {'shared': {'ipc'}, 'sockets': {'x11', 'wayland', 'fallback-x11'},
               'devices': {'dri'}}
    for key, value in metadata.items('Context'):
        if key not in allowed or set(filter(None, value.split(';'))) - allowed[key]:
            raise ValueError(f'Unexpected Flatpak permission: {key}')
    for section in ('Session Bus Policy', 'System Bus Policy'):
        if metadata.has_section(section) and list(metadata.items(section)):
            raise ValueError(f'Unexpected Flatpak bus permission: {section}')
    if dict(metadata.items('Environment')) != {'QT_QPA_PLATFORMTHEME': 'xdgdesktopportal'}:
        raise ValueError('Unexpected Flatpak environment')
    return {section: dict(metadata.items(section)) for section in ('Context', 'Environment')}


def check_commit(actual, expected):
    if not re.fullmatch(r'[0-9a-f]{64}', expected) or actual != expected:
        raise ValueError('Installed Flatpak dependency differs from reviewed lock')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--work', type=Path, default=ROOT / 'build/flatpak')
    parser.add_argument('--suffix', default='dev')
    parser.add_argument('--jobs', type=int, default=2)
    args = parser.parse_args()
    if not re.fullmatch(r'[0-9A-Za-z]+([.-][0-9A-Za-z]+)*', args.suffix) or args.jobs < 1:
        parser.error('Use a valid prerelease suffix and positive job count')
    if output('git', '-C', str(ROOT), 'status', '--porcelain', '--untracked-files=no'):
        raise ValueError('Commit tracked changes before packaging')
    work = args.work.resolve()
    work.mkdir(parents=True, exist_ok=False)
    lock = json.loads((ROOT / 'packaging/flatpak-runtime.json').read_text())
    dependencies = {}
    for kind in ('runtime', 'sdk'):
        entry = lock[kind]
        ref = f"{entry['id']}/{lock['architecture']}/{lock['branch']}"
        commit = output('flatpak', 'info', '--user', '--show-commit', ref)
        check_commit(commit, entry['commit'])
        dependencies[kind] = {'ref': ref, 'commit': commit}
    commit = output('git', '-C', str(ROOT), 'rev-parse', 'HEAD')
    source = work / 'openpaint-source.tar'
    run('git', '-C', str(ROOT), 'archive', '--format=tar', '--prefix=openpaint/',
        f'--output={source}', commit)
    manifest = json.loads((ROOT / 'packaging/io.github.AfshinKI.OpenPaint.json').read_text())
    module = manifest['modules'][0]
    module['config-opts'] = [option for option in module['config-opts']
                             if not option.startswith('-DOPENPAINT_VERSION_SUFFIX=')]
    module['config-opts'].append(f'-DOPENPAINT_VERSION_SUFFIX={args.suffix}')
    module['sources'][0]['sha256'] = digest(source)
    manifest_path = work / 'manifest.json'
    manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
    run('flatpak-builder', '--user', '--disable-download', '--sandbox',
        f'--jobs={args.jobs}', f'--state-dir={work / "state"}', f'--repo={work / "repo"}',
        str(work / 'app'), str(manifest_path))
    permissions = audit_permissions((work / 'app/metadata').read_text())
    binary = work / 'app/files/bin/openpaint'
    for prefix in (str(ROOT).encode(), str(Path.home()).encode()):
        if len(prefix) > 5 and prefix in binary.read_bytes():
            raise ValueError('Local build path in Flatpak executable')
    version = re.search(r'project\(OpenPaint VERSION ([0-9.]+)', (ROOT / 'CMakeLists.txt').read_text())[1]
    bundle = work / f'OpenPaint-{version}-{args.suffix}-{lock["architecture"]}.flatpak'
    run('flatpak', 'build-bundle', f'--runtime-repo={lock["remote"]}',
        str(work / 'repo'), str(bundle), APP_ID, 'alpha')
    inventory = {'application': APP_ID, 'version': f'{version}-{args.suffix}',
                 'source_commit': commit, 'source_sha256': digest(source),
                 'bundle_sha256': digest(bundle), 'dependencies': dependencies,
                 'permissions': permissions, 'flatpak': output('flatpak', '--version'),
                 'builder': output('flatpak-builder', '--version'),
                 'runtime_source': 'https://invent.kde.org/packaging/flatpak-kde-runtime',
                 'runtime_distributed_separately': True, 'signed_by_openpaint': False}
    inventory_path = work / 'flatpak-inventory.json'
    inventory_path.write_text(json.dumps(inventory, indent=2) + '\n')
    artifacts = [bundle, source, inventory_path, manifest_path]
    (work / 'SHA256SUMS').write_text(''.join(f'{digest(path)}  {path.name}\n' for path in artifacts))
    print(f'Built {bundle.name}; no release was published or app installed.')


if __name__ == '__main__':
    main()
