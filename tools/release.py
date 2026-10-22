#!/usr/bin/env python3
"""Select a release identity and stage verified same-run Linux and macOS artifacts."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import re
import shutil
import tarfile

ROOT = Path(__file__).resolve().parents[1]
VERSION = re.compile(r'(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)')


def identity(requested, source_version, event, ref, run_number):
    if not VERSION.fullmatch(source_version):
        raise ValueError('Invalid source version')
    if requested:
        if (event != 'workflow_dispatch' or ref != 'refs/heads/main'
                or not VERSION.fullmatch(requested) or requested != source_version):
            raise ValueError('Release requires workflow dispatch on main and exact source version')
        return {'version': requested, 'suffix': '', 'publish': 'true'}
    if not run_number.isdigit():
        raise ValueError('Invalid run number')
    return {'version': f'{source_version}-dev.{run_number}',
            'suffix': f'dev.{run_number}', 'publish': 'false'}


def digest(path):
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def verify(directory, expected):
    entries = {}
    for line in (directory / 'SHA256SUMS').read_text().splitlines():
        match = re.fullmatch(r'([0-9a-f]{64})  ([A-Za-z0-9_.-]+)', line)
        if not match or match[2] in entries:
            raise ValueError('Invalid or duplicate checksum entry')
        entries[match[2]] = match[1]
    if set(entries) != set(expected):
        raise ValueError('Release files differ from expected version and format')
    for name, expected_hash in entries.items():
        path = directory / name
        if path.is_symlink() or not path.is_file() or digest(path) != expected_hash:
            raise ValueError(f'Checksum mismatch or missing file: {name}')


def stage(artifacts, destination, version, commit):
    if not VERSION.fullmatch(version) or not re.fullmatch(r'[0-9a-f]{40}', commit):
        raise ValueError('Invalid stable version or source commit')
    compliance = f'OpenPaint-{version}-sources-and-notices.tar'
    binaries = {'portable': f'OpenPaint-{version}-linux-x86_64.tar.gz',
                'deb': f'OpenPaint-{version}-ubuntu24.04-amd64.deb',
                'flatpak': f'OpenPaint-{version}-x86_64.flatpak',
                'macos': f'OpenPaint-{version}-macos-arm64.dmg'}
    mac_compliance = f'OpenPaint-{version}-macos-sources-and-notices.tar'
    flat_sources = ['openpaint-source.tar', 'flatpak-inventory.json', 'manifest.json']
    for kind, binary in binaries.items():
        verify(artifacts / kind, [binary] + (flat_sources if kind == 'flatpak' else
               [mac_compliance] if kind == 'macos' else [compliance]))
    for kind in ('portable', 'deb'):
        with tarfile.open(artifacts / kind / compliance) as archive:
            native = json.load(archive.extractfile('dependency-inventory.json'))
        if native['version'] != version or native['source_commit'] != commit:
            raise ValueError('Native provenance differs from release')
    inventory = json.loads((artifacts / 'flatpak/flatpak-inventory.json').read_text())
    if (inventory['version'] != version or inventory['source_commit'] != commit
            or inventory['bundle_sha256'] != digest(artifacts / 'flatpak' / binaries['flatpak'])
            or inventory['source_sha256'] != digest(artifacts / 'flatpak/openpaint-source.tar')):
        raise ValueError('Flatpak provenance differs from release')
    with tarfile.open(artifacts / 'macos' / mac_compliance) as archive:
        mac = json.load(archive.extractfile('macos-inventory.json'))
    if (mac['version'] != version or mac['source_commit'] != commit
            or mac['architecture'] != 'arm64'
            or mac['bundle_sha256'] != digest(artifacts / 'macos' / binaries['macos'])):
        raise ValueError('macOS provenance differs from release')
    destination.mkdir(parents=True, exist_ok=False)
    for kind, binary in binaries.items():
        shutil.copyfile(artifacts / kind / binary, destination / binary)
    # Keep format-specific inventories beside their matching source bundles.
    with tarfile.open(destination / compliance, 'w') as archive:
        for kind in ('portable', 'deb'):
            archive.add(artifacts / kind / compliance, arcname=f'{kind}/{compliance}')
        archive.add(artifacts / 'macos' / mac_compliance, arcname=f'macos/{mac_compliance}')
        for name in flat_sources:
            archive.add(artifacts / 'flatpak' / name, arcname=f'flatpak/{name}')
    paths = sorted(destination.iterdir())
    (destination / 'SHA256SUMS').write_text(''.join(f'{digest(p)}  {p.name}\n' for p in paths))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest='command', required=True)
    sub.add_parser('identity')
    staging = sub.add_parser('stage')
    staging.add_argument('--artifacts', type=Path, required=True)
    staging.add_argument('--destination', type=Path, required=True)
    staging.add_argument('--version', required=True)
    staging.add_argument('--commit', required=True)
    args = parser.parse_args()
    if args.command == 'identity':
        source_version = re.search(r'project\(OpenPaint VERSION ([0-9.]+)',
                                   (ROOT / 'CMakeLists.txt').read_text())[1]
        values = identity(os.environ.get('RELEASE_VERSION', ''), source_version,
                          os.environ['GITHUB_EVENT_NAME'], os.environ['GITHUB_REF'],
                          os.environ['GITHUB_RUN_NUMBER'])
        with open(os.environ['GITHUB_OUTPUT'], 'a') as output:
            output.write(''.join(f'{key}={value}\n' for key, value in values.items()))
    else:
        stage(args.artifacts, args.destination, args.version, args.commit)


if __name__ == '__main__':
    main()
