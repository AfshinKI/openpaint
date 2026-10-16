#!/bin/bash
# Run only in a disposable Ubuntu test environment; never on the user's desktop.
set -euo pipefail
package=$(realpath "$1")
version=$(dpkg-deb -f "$package" Version)
fixture=$(mktemp -d)
trap 'rm -rf "$fixture"' EXIT
# A generated user file outside package-owned paths must survive replacement/removal.
python3 - "$fixture/artwork.png" <<'PYPNG'
import struct, sys, zlib
from pathlib import Path
def chunk(kind, data):
    return struct.pack('!I', len(data)) + kind + data + struct.pack('!I', zlib.crc32(kind + data))
Path(sys.argv[1]).write_bytes(b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('!2I5B', 1, 1, 8, 2, 0, 0, 0)) + chunk(b'IDAT', zlib.compress(b'\x00\xff\x00\x00')) + chunk(b'IEND', b''))
PYPNG
before=$(sha256sum "$fixture/artwork.png")
# Install a lower-version fixture with the same payload, then exercise APT upgrade.
# This tests package transactions, not compatibility with a historical app binary.
dpkg-deb -R "$package" "$fixture/previous"
sed -i "s/^Version:.*/Version: 0.0.0-1/" "$fixture/previous/DEBIAN/control"
dpkg-deb --root-owner-group --build "$fixture/previous" "$fixture/previous.deb"
chmod 755 "$fixture"
sudo apt-get install -y "$fixture/previous.deb"
sudo apt-get install -y "$package"
test "$(dpkg-query -W -f='${Version}' openpaint)" = "$version"
desktop-file-validate /usr/share/applications/io.github.AfshinKI.OpenPaint.desktop
test -f /usr/share/icons/hicolor/scalable/apps/io.github.AfshinKI.OpenPaint.svg
test -f /usr/share/metainfo/io.github.AfshinKI.OpenPaint.metainfo.xml
QT_QPA_PLATFORM=offscreen openpaint --version
grep -q 'io.github.AfshinKI.OpenPaint.desktop' /usr/share/applications/mimeinfo.cache
# Exercise the installed launcher and X11 plugin without any build-tree Qt.
xvfb-run -a sh -c '
  openpaint &
  app=$!
  sleep 5
  kill -0 "$app"
  kill "$app"
  wait "$app" || test "$?" = 143
'
sudo apt-get install -y --reinstall "$package"
test "$(sha256sum "$fixture/artwork.png")" = "$before"
sudo apt-get remove -y openpaint
test ! -e /usr/bin/openpaint
test ! -e /usr/lib/openpaint
test ! -e /usr/share/applications/io.github.AfshinKI.OpenPaint.desktop
test ! -e /usr/share/icons/hicolor/scalable/apps/io.github.AfshinKI.OpenPaint.svg
test "$(sha256sum "$fixture/artwork.png")" = "$before"
printf 'Installed, upgraded, launched, reinstalled and removed successfully; artwork preserved.\n'
