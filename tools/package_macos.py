#!/usr/bin/env python3
"""Bundle and audit an already-tested macOS build; never publish or install."""
import argparse
import json
import os
from pathlib import Path
import platform
import re
import shutil
import subprocess
import tarfile
import tempfile

from package_appimage import digest, fetch, license_files, public_tar_info

ROOT = Path(__file__).resolve().parents[1]


def stage_plugins(qt, bundle):
    """Select GUI/image/network plugins without unused external database clients."""
    selected = [qt / 'plugins/platforms/libqcocoa.dylib']
    for kind in ('imageformats', 'iconengines', 'styles', 'printsupport',
                 'tls', 'networkinformation', 'platforminputcontexts'):
        selected.extend(sorted((qt / 'plugins' / kind).glob('*.dylib')))
    # macdeployqt also copies QtQuick.LocalStorage; retain its self-contained driver.
    sqlite = qt / 'plugins/sqldrivers/libqsqlite.dylib'
    if sqlite.is_file():
        selected.append(sqlite)
    deployed = []
    for source in selected:
        if source.name.endswith('_debug.dylib'):
            continue
        target = bundle / 'Contents/PlugIns' / source.relative_to(qt / 'plugins')
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        deployed.append(target)
    return deployed


def audit_bundle(bundle):
    records = []
    for path in sorted(bundle.rglob('*')):
        if path.is_symlink():
            if not path.resolve().is_relative_to(bundle.resolve()):
                raise ValueError('Bundle symlink escapes application')
            continue
        if not path.is_file():
            continue
        data = path.read_bytes()
        if str(ROOT).encode() in data:
            raise ValueError('Build path leaked into bundle: ' + str(path.relative_to(bundle)))
        if data[:4] in (b'\xcf\xfa\xed\xfe', b'\xce\xfa\xed\xfe', b'\xca\xfe\xba\xbe',
                         b'\xbe\xba\xfe\xca', b'\xfe\xed\xfa\xcf'):
            output = subprocess.check_output(['otool', '-L', str(path)], text=True)
            for line in output.splitlines():
                # Universal binaries repeat unindented per-architecture file headings.
                if not line or not line[0].isspace():
                    continue
                dependency = line.strip().split(' (')[0]
                if not dependency.startswith(('@rpath/', '@loader_path/', '@executable_path/',
                                               '/System/Library/', '/usr/lib/')):
                    raise ValueError('External runtime dependency in ' + str(path.relative_to(bundle))
                                     + ': ' + dependency)
        records.append({'path': str(path.relative_to(bundle)), 'bytes': len(data), 'sha256': digest(path)})
    if not (bundle / 'Contents/PlugIns/platforms/libqcocoa.dylib').is_file():
        raise ValueError('Missing Cocoa platform plugin')
    return records


def smoke_fresh_copy(bundle):
    """Launch the deployed app via Cocoa from a fresh path, without editing artwork."""
    with tempfile.TemporaryDirectory(prefix='openpaint-package-') as temporary:
        installed = Path(temporary) / 'OpenPaint.app'
        shutil.copytree(bundle, installed, symlinks=True)
        with tempfile.TemporaryFile() as output:
            process = subprocess.Popen([str(installed / 'Contents/MacOS/OpenPaint')],
                env=dict(os.environ, QT_QPA_PLATFORM='cocoa', QT_QUICK_BACKEND='rhi',
                         QSG_RHI_BACKEND='metal',
                         XDG_CONFIG_HOME=str(Path(temporary) / 'settings')),
                stdout=output, stderr=output)
            try:
                try:
                    process.wait(timeout=8)
                except subprocess.TimeoutExpired:
                    pass
                else:
                    raise ValueError('Fresh-copy Cocoa application exited during startup')
            finally:
                process.terminate()
                try:
                    process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
            output.seek(0)
            log = output.read().decode(errors='replace')
            if any(message in log for message in ('failed to load component',
                    'module is not installed', 'Could not load the Qt platform plugin',
                    'Failed to create', 'Symbol not found')):
                raise ValueError('Fresh-copy Cocoa startup reported a runtime failure')
    print('Fresh-copy Cocoa startup smoke passed; visual appearance requires separate review.')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--qt', type=Path, required=True)
    parser.add_argument('--build', type=Path, default=ROOT / 'build/release')
    parser.add_argument('--version', required=True)
    args = parser.parse_args()
    if platform.system() != 'Darwin' or platform.machine() != 'arm64':
        raise SystemExit('This recipe requires a native Apple Silicon Mac')
    if not re.fullmatch(r'\d+\.\d+\.\d+(?:-[A-Za-z0-9.-]+)?', args.version):
        raise ValueError('Invalid version')
    work = ROOT / 'build/macos'
    public = work / 'public'
    stage = work / 'stage'
    if work.exists():
        shutil.rmtree(work)
    public.mkdir(parents=True)
    stage.mkdir()
    bundle = stage / 'OpenPaint.app'
    shutil.copytree(args.build / 'OpenPaint.app', bundle, symlinks=True)
    plugins = stage_plugins(args.qt, bundle)
    print('Deploying GUI, image and network plugins with Qt/QML dependencies.', flush=True)
    subprocess.run([str(args.qt / 'bin/macdeployqt'), str(bundle),
                    '-qmldir=' + str(ROOT / 'qml'), '-always-overwrite', '-no-plugins',
                    '-verbose=1', *['-executable=' + str(p) for p in plugins]], check=True)
    iconset = work / 'OpenPaint.iconset'
    subprocess.run([str(args.build / 'openpaint_icon_tool'), str(ROOT / 'qml/icons/app.svg'),
                    str(iconset)], env=dict(os.environ, QT_QPA_PLATFORM='offscreen'), check=True)
    resources = bundle / 'Contents/Resources'
    resources.mkdir(exist_ok=True)
    subprocess.run(['iconutil', '-c', 'icns', str(iconset),
                    '-o', str(resources / 'OpenPaint.icns')], check=True)
    # Ad-hoc signatures provide bundle integrity on Apple Silicon, not Developer ID trust.
    subprocess.run(['codesign', '--force', '--deep', '--sign', '-', str(bundle)], check=True)
    subprocess.run(['codesign', '--verify', '--deep', '--strict', str(bundle)], check=True)
    actual = subprocess.check_output([str(bundle / 'Contents/MacOS/OpenPaint'), '--version'],
                                     env=dict(os.environ, QT_QPA_PLATFORM='cocoa'), text=True).strip()
    if actual != 'OpenPaint ' + args.version:
        raise ValueError('Bundle version mismatch: ' + actual)
    files = audit_bundle(bundle)
    smoke_fresh_copy(bundle)
    (stage / 'Applications').symlink_to('/Applications')
    dmg = public / f'OpenPaint-{args.version}-macos-arm64.dmg'
    subprocess.run(['hdiutil', 'create', '-volname', 'OpenPaint', '-srcfolder', str(stage),
                    '-ov', '-format', 'UDZO', str(dmg)], check=True)
    sources = work / 'sources'
    sources.mkdir()
    notices = work / 'notices'
    notices.mkdir()
    upstream = [entry for entry in json.loads((ROOT / 'packaging/appimage-sources.json').read_text())
                if entry['name'].startswith(('qtbase-', 'qtdeclarative-', 'qtsvg-', 'qtimageformats-'))]
    for entry in upstream:
        source = fetch(entry, ROOT / 'build/download-cache')
        shutil.copyfile(source, sources / source.name)
        license_files(source, notices / source.name)
    subprocess.run(['git', 'archive', '--format=tar', '--output=' + str(sources / 'openpaint-source.tar'),
                    'HEAD'], cwd=ROOT, check=True)
    inventory = dict(version=args.version, source_commit=subprocess.check_output(
        ['git', 'rev-parse', 'HEAD'], cwd=ROOT, text=True).strip(), architecture='arm64',
        minimum_macos='14.4', signing='ad-hoc; not Developer ID signed or notarized',
        bundle_sha256=digest(dmg), upstreamSources=upstream, files=files,
        system_dependencies='Apple SDK libarchive, zlib and system frameworks; not redistributed')
    (work / 'macos-inventory.json').write_text(json.dumps(inventory, indent=2) + '\n')
    compliance = public / f'OpenPaint-{args.version}-macos-sources-and-notices.tar'
    with tarfile.open(compliance, 'w') as output:
        for path in (sources, notices, work / 'macos-inventory.json', ROOT / 'LICENSE'):
            output.add(path, arcname=path.name, filter=public_tar_info)
    (public / 'SHA256SUMS').write_text(''.join(f'{digest(p)}  {p.name}\n'
                                             for p in sorted(public.iterdir())))
    print('Audited macOS bundle, sources and disk image staged.')


if __name__ == '__main__':
    main()
