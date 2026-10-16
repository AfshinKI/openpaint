#!/usr/bin/env python3
"""Stage an Ubuntu amd64 DEB from an audited, system-identity AppDir."""
from __future__ import annotations

import hashlib
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess

APP_ID = 'io.github.AfshinKI.OpenPaint'


def deb_version(version: str) -> str:
    if not re.fullmatch(r'(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)\.(0|[1-9][0-9]*)(-[0-9A-Za-z]+([.-][0-9A-Za-z]+)*)?', version):
        raise ValueError('Invalid release version')
    # Debian sorts prereleases before the matching stable release.
    return version.replace('-', '~', 1) + '-1'


def elf_files(root: Path) -> list[Path]:
    files = []
    for path in sorted(root.rglob('*')):
        if path.is_file() and not path.is_symlink():
            with path.open('rb') as stream:
                if stream.read(4) == b'\x7fELF':
                    files.append(path)
    return files


def dependencies(stage: Path, work: Path) -> str:
    binaries = elf_files(stage)
    if not binaries:
        raise ValueError('No ELF runtime found')
    # Local shlibs entries describe only libraries actually shipped in this DEB.
    entries = set()
    for binary in binaries:
        dynamic = subprocess.check_output(['readelf', '-d', str(binary)], text=True)
        for soname in re.findall(r'\(SONAME\).*?\[(.*?)\]', dynamic):
            match = re.fullmatch(r'(.+)\.so\.(.+)', soname)
            if match:
                entries.add(f'{match[1]} {match[2]} openpaint\n')
    local = work / 'shlibs.local'
    local.write_text(''.join(sorted(entries)))
    control_dir = work / 'debian'
    control_dir.mkdir()
    (control_dir / 'control').write_text('Source: openpaint\n\nPackage: openpaint\nArchitecture: amd64\nDescription: Offline paint editor\n')
    result = subprocess.check_output([
        'dpkg-shlibdeps', '-O', '-xopenpaint', '-L' + str(local),
        '-l' + str(stage / 'usr/lib/openpaint/usr/lib'),
        *[str(binary) for binary in binaries],
    ], cwd=work, text=True)
    match = re.search(r'^shlibs:Depends=(.+)$', result, re.MULTILINE)
    if not match:
        raise ValueError('Missing generated system dependencies')
    return match[1] + ', desktop-file-utils, shared-mime-info, hicolor-icon-theme, xdg-desktop-portal'


def build_deb(appdir: Path, work: Path, version: str) -> Path:
    version_deb = deb_version(version)
    release = platform.freedesktop_os_release()
    if (release.get('ID'), release.get('VERSION_ID')) != ('ubuntu', '24.04'):
        raise ValueError('Build this package on Ubuntu 24.04')
    if subprocess.check_output(['dpkg', '--print-architecture'], text=True).strip() != 'amd64':
        raise ValueError('Only amd64 is validated by this recipe')
    stage = work / 'deb-root'
    stage.mkdir()  # Refuse mixed or stale staging trees.
    stage.chmod(0o755)
    private = stage / 'usr/lib/openpaint'
    shutil.copytree(appdir, private, symlinks=True)
    for path in private.rglob('*'):
        if path.is_symlink() and not path.resolve().is_relative_to(private.resolve()):
            raise ValueError('Package symlink escapes private runtime')
    binary = stage / 'usr/bin/openpaint'
    binary.parent.mkdir(parents=True)
    binary.write_text('#!/bin/sh\nexport QT_QPA_PLATFORMTHEME="${QT_QPA_PLATFORMTHEME:-xdgdesktopportal}"\nexec /usr/lib/openpaint/usr/bin/openpaint "$@"\n')
    binary.chmod(0o755)
    for relative in [f'applications/{APP_ID}.desktop', f'metainfo/{APP_ID}.metainfo.xml',
                     f'icons/hicolor/scalable/apps/{APP_ID}.svg']:
        target = stage / 'usr/share' / relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(appdir / 'usr/share' / relative, target)
    docs = stage / 'usr/share/doc/openpaint'
    docs.mkdir(parents=True)
    shutil.copyfile(appdir / f'usr/share/licenses/{APP_ID}/LICENSE', docs / 'copyright')
    control = stage / 'DEBIAN'
    control.mkdir()
    depends = dependencies(stage, work)
    size = sum(p.stat().st_size for p in stage.rglob('*') if p.is_file() and not p.is_symlink())
    (control / 'control').write_text(
        f'Package: openpaint\nVersion: {version_deb}\nArchitecture: amd64\n'
        'Maintainer: OpenPaint contributors <51800314+AfshinKI@users.noreply.github.com>\n'
        'Section: graphics\nPriority: optional\n'
        f'Installed-Size: {(size + 1023) // 1024}\nDepends: {depends}\n'
        'Homepage: https://github.com/AfshinKI/openpaint\n'
        'Description: Offline desktop paint editor\n'
        ' Drawing, selections, text and image transformations with private Qt libraries.\n')
    # Normalize modes regardless of the builder's umask. Preserve executable bits.
    for path in stage.rglob('*'):
        if not path.is_symlink():
            path.chmod(0o755 if path.is_dir() or path.stat().st_mode & 0o111 else 0o644)
    output = work / f'OpenPaint-{version}-ubuntu24.04-amd64.deb'
    subprocess.run(['dpkg-deb', '--root-owner-group', '--build', str(stage), str(output)], check=True,
                   env=dict(os.environ, SOURCE_DATE_EPOCH=os.environ.get('SOURCE_DATE_EPOCH', '0')))
    inventory = {'package': output.name, 'version': version_deb, 'architecture': 'amd64',
                 'depends': depends, 'files': []}
    for path in sorted(stage.rglob('*')):
        if path.is_symlink():
            inventory['files'].append({'path': str(path.relative_to(stage)), 'link': os.readlink(path)})
        elif path.is_file():
            with path.open('rb') as stream:
                sha = hashlib.file_digest(stream, 'sha256').hexdigest()
            inventory['files'].append({'path': str(path.relative_to(stage)), 'sha256': sha})
    with output.open('rb') as stream:
        inventory['sha256'] = hashlib.file_digest(stream, 'sha256').hexdigest()
    (work / 'deb-inventory.json').write_text(json.dumps(inventory, indent=2) + '\n')
    return output
