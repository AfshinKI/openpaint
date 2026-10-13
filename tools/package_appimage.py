#!/usr/bin/env python3
"""Build an audited AppImage on Ubuntu 24.04; never publish or install it."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import urllib.request

ROOT = Path(__file__).resolve().parents[1]


def digest(path: Path) -> str:
    with path.open('rb') as stream:
        return hashlib.file_digest(stream, 'sha256').hexdigest()


def fetch(entry: dict, cache: Path) -> Path:
    target = cache / entry['name']
    if not target.exists():
        temporary = target.with_suffix(target.suffix + '.partial')
        try:
            with urllib.request.urlopen(entry['url'], timeout=120) as response, temporary.open('wb') as out:
                shutil.copyfileobj(response, out)
            if digest(temporary) != entry['sha256']:
                raise ValueError('Download digest mismatch: ' + entry['name'])
            temporary.replace(target)
        finally:
            temporary.unlink(missing_ok=True)
    if digest(target) != entry['sha256']:
        raise ValueError('Cached digest mismatch: ' + entry['name'])
    return target


def license_files(archive: Path, destination: Path) -> None:
    """Copy only regular license/attribution text; never extract archive links."""
    with tarfile.open(archive) as source:
        for member in source:
            path = Path(member.name)
            if path.is_absolute() or '..' in path.parts:
                raise ValueError('Unsafe source archive path')
            name = path.name.lower()
            if not member.isfile() or not (name.startswith(('license', 'copying')) or name == 'qt_attribution.json'):
                continue
            if member.size > 4 * 1024 * 1024:
                raise ValueError('Oversized license file')
            target = destination / path
            target.parent.mkdir(parents=True, exist_ok=True)
            with source.extractfile(member) as src, target.open('wb') as out:
                shutil.copyfileobj(src, out)


def audit(appdir: Path, private_prefixes: list[bytes]) -> list[dict]:
    records = []
    for path in sorted(appdir.rglob('*')):
        if path.is_symlink():
            if not path.resolve().is_relative_to(appdir.resolve()):
                raise ValueError('Package symlink escapes staging directory')
            continue
        if not path.is_file():
            continue
        data = path.read_bytes()
        if any(prefix and prefix in data for prefix in private_prefixes):
            raise ValueError('Local build path found in ' + str(path.relative_to(appdir)))
        records.append({'path': str(path.relative_to(appdir)), 'sha256': digest(path), 'bytes': len(data)})
    for required in ['usr/bin/openpaint', 'usr/plugins/platforms/libqwayland.so',
                     'usr/plugins/platforms/libqxcb.so', 'usr/plugins/platformthemes/libqxdgdesktopportal.so',
                     'usr/plugins/imageformats/libqwebp.so',
                     'usr/plugins/wayland-graphics-integration-client/libqt-plugin-wayland-egl.so',
                     'usr/plugins/wayland-shell-integration/libxdg-shell.so']:
        if not (appdir / required).is_file():
            raise ValueError('Missing runtime component: ' + required)
    return records


def system_packages(appdir: Path) -> list[dict]:
    result = {}
    for library in sorted((appdir / 'usr/lib').glob('*.so*')):
        if library.name.startswith(('libQt6', 'libicu')):
            continue
        matches = subprocess.run(['dpkg-query', '-S', '*/' + library.name], capture_output=True, text=True, check=True)
        packages = sorted({line.split(': /')[0] for line in matches.stdout.splitlines() if ': /' in line})
        packages = [package for package in packages if not ':' in package or package.endswith(':amd64')]
        if len(packages) != 1:
            raise ValueError('Cannot identify dependency: ' + library.name)
        package = packages[0]
        fields = subprocess.check_output(['dpkg-query', '-W', '-f=${binary:Package}\t${Version}\t${source:Package}\t${source:Version}', package], text=True).split('\t')
        result[package] = dict(zip(['package', 'version', 'source', 'sourceVersion'], fields))
        copyright_file = Path('/usr/share/doc') / package.split(':')[0] / 'copyright'
        if not copyright_file.is_file():
            raise ValueError('Missing dependency copyright: ' + package)
        target = appdir / 'usr/share/licenses/dependencies' / package.split(':')[0]
        target.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(copyright_file, target / 'copyright')
    return list(result.values())


def public_tar_info(info: tarfile.TarInfo) -> tarfile.TarInfo:
    info.uid = info.gid = 0
    info.uname = info.gname = ''
    info.mtime = 0
    return info


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--qt', type=Path, required=True)
    parser.add_argument('--work', type=Path, default=ROOT / 'build/appimage')
    parser.add_argument('--suffix', default='dev')
    parser.add_argument('--jobs', type=int, default=2)
    parser.add_argument('--collect-system-sources', action='store_true', help='Requires signed deb-src indexes; mandatory before release')
    args = parser.parse_args()
    work = args.work.resolve()
    qt = args.qt.resolve()
    if subprocess.check_output([str(qt / 'bin/qmake'), '-query', 'QT_VERSION'], text=True).strip() != '6.12.0':
        raise ValueError('This recipe requires Qt 6.12.0')
    if (work / 'AppDir').exists():
        raise ValueError('Use a fresh work directory; refusing to mix package builds')
    work.mkdir(parents=True, exist_ok=True)
    cache = ROOT / 'build/package-cache'
    cache.mkdir(parents=True, exist_ok=True)
    tools = json.loads((ROOT / 'packaging/appimage-tools.json').read_text())
    for entry in tools:
        fetch(entry, cache).chmod(0o755)
    sources = json.loads((ROOT / 'packaging/appimage-sources.json').read_text())
    appdir = work / 'AppDir'
    env = dict(os.environ, APPIMAGE_EXTRACT_AND_RUN='1', ARCH='x86_64',
               PATH=str(cache) + os.pathsep + os.environ['PATH'],
               LD_LIBRARY_PATH=str(qt / 'lib'), QMAKE=str(qt / 'bin/qmake'),
               QML_SOURCES_PATHS=str(ROOT / 'qml'), EXTRA_QT_MODULES='svg',
               EXTRA_PLATFORM_PLUGINS='libqwayland.so;libqoffscreen.so')
    def run(command, **kwargs):
        subprocess.run([str(part) for part in command], check=True, env=env, **kwargs)
    run(['cmake', '--fresh', '-S', ROOT, '-B', work / 'compile', '-G', 'Ninja', '-DCMAKE_BUILD_TYPE=Release',
         '-DBUILD_TESTING=OFF', '-DCMAKE_PREFIX_PATH=' + str(qt), '-DOPENPAINT_INSTALL_KIND=appimage',
         '-DOPENPAINT_VERSION_SUFFIX=' + args.suffix])
    run(['cmake', '--build', work / 'compile', '--parallel', args.jobs])
    install_env = dict(env, DESTDIR=str(appdir))
    subprocess.run(['cmake', '--install', str(work / 'compile'), '--prefix', '/usr', '--strip'], env=install_env, check=True)
    # Qt 6.12 Wayland helpers are not discovered by this linuxdeploy Qt plugin.
    for directory in ['wayland-graphics-integration-client', 'wayland-shell-integration', 'wayland-decoration-client']:
        shutil.copytree(qt / 'plugins' / directory, appdir / 'usr/plugins' / directory)
    run([cache / 'linuxdeploy-x86_64.AppImage', '--appdir', appdir, '--plugin', 'qt',
         '--library', '/usr/lib/x86_64-linux-gnu/libOpenGL.so.0'])
    launcher = appdir / 'AppRun'
    launcher.unlink()
    shutil.copyfile(ROOT / 'packaging/AppRun', launcher)
    launcher.chmod(0o755)
    licenses = appdir / 'usr/share/licenses/dependencies'
    source_dir = work / 'sources'
    source_dir.mkdir()
    for entry in sources:
        archive = fetch(entry, cache)
        license_files(archive, licenses)
        shutil.copyfile(archive, source_dir / archive.name)
    packages = system_packages(appdir)
    if args.collect_system_sources:
        for name, version in sorted({(p['source'], p['sourceVersion']) for p in packages}):
            run(['apt-get', 'source', '--download-only', name + '=' + version], cwd=source_dir)
    shutil.copytree('/usr/share/common-licenses', licenses / 'common-licenses', dirs_exist_ok=True)
    notices = json.loads((ROOT / 'packaging/appimage-notices.json').read_text())
    for entry in notices:
        notice = fetch(entry, cache)
        shutil.copyfile(notice, licenses / notice.name)
        shutil.copyfile(notice, source_dir / notice.name)
    # Upstream metadata uses the current .metainfo suffix; older appimagetool also expects .appdata.
    metadata = appdir / 'usr/share/metainfo'
    (metadata / 'io.github.AfshinKI.OpenPaint.appdata.xml').symlink_to('io.github.AfshinKI.OpenPaint.metainfo.xml')
    version_env = dict(env, QT_QPA_PLATFORM='offscreen')
    version = subprocess.check_output([str(appdir / 'AppRun'), '--version'], env=version_env, text=True).strip().split()[-1]
    # A controlled root-level /src checkout is not a private developer path.
    private_prefixes = [] if Path.home() == Path('/root') else [str(Path.home()).encode()]
    if len(ROOT.parts) > 2:
        private_prefixes.append(str(ROOT).encode())
    files = audit(appdir, private_prefixes)
    inventory = {'version': version, 'architecture': 'x86_64', 'qt': '6.12.0', 'icu': '73.2',
                 'tools': tools, 'upstreamSources': sources, 'notices': notices, 'systemPackages': packages,
                 'systemSourcesIncluded': args.collect_system_sources, 'files': files}
    (work / 'dependency-inventory.json').write_text(json.dumps(inventory, indent=2) + '\n')
    shutil.copyfile(work / 'dependency-inventory.json', appdir / 'usr/share/doc/OpenPaint/dependency-inventory.json')
    image = work / f'OpenPaint-{version}-x86_64.AppImage'
    run([cache / 'appimagetool-x86_64.AppImage', '--runtime-file', cache / 'runtime-x86_64', appdir, image])
    source_bundle = work / f'OpenPaint-{version}-third-party-sources.tar'
    with tarfile.open(source_bundle, 'w') as archive:
        for source in sorted(source_dir.iterdir()):
            if source.is_file():
                archive.add(source, arcname=source.name, filter=public_tar_info)
        archive.add(work / 'dependency-inventory.json', arcname='dependency-inventory.json', filter=public_tar_info)
    components = [{'type': 'framework', 'name': 'Qt', 'version': '6.12.0'},
                  {'type': 'library', 'name': 'ICU', 'version': '73.2'},
                  {'type': 'library', 'name': 'AppImage type2-runtime',
                   'version': tools[-1]['source'].rsplit('/', 1)[-1],
                   'hashes': [{'alg': 'SHA-256', 'content': tools[-1]['sha256']}]}]
    components.extend({'type': 'library', 'name': p['package'], 'version': p['version'],
                       'properties': [{'name': 'ubuntu:source', 'value': p['source']},
                                      {'name': 'ubuntu:source-version', 'value': p['sourceVersion']}]}
                      for p in packages)
    bom = {'bomFormat': 'CycloneDX', 'specVersion': '1.6', 'version': 1,
           'metadata': {'component': {'type': 'application', 'name': 'OpenPaint', 'version': version,
                                     'hashes': [{'alg': 'SHA-256', 'content': digest(image)}]}},
           'components': components}
    bom_path = work / 'bom.cdx.json'
    bom_path.write_text(json.dumps(bom, indent=2) + '\n')
    (work / 'SHA256SUMS').write_text(''.join(digest(path) + '  ' + path.name + '\n'
                                          for path in [image, source_bundle, work / 'dependency-inventory.json', bom_path]))
    print('Built ' + image.name + '; no release was published.')


if __name__ == '__main__':
    main()
