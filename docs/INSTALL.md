# OpenPaint installation

Download assets from [GitHub Releases](https://github.com/AfshinKI/openpaint/releases).

Linux x86_64. Tested release environments and artifact digests are recorded in the
release evidence. Keep copies of important originals; crash recovery is not implemented.

## Verify downloads

Download `SHA256SUMS` and the desired assets from the same GitHub release. In that
directory, run `sha256sum --ignore-missing -c SHA256SUMS`. Every downloaded entry
must report OK. Checksums detect changed bytes; these artifacts do not have
an OpenPaint signing signature or an automatic updater.

## Ubuntu 24.04 installer

The DEB recipe targets Ubuntu 24.04 amd64. The bundled Qt runtime requires
SSSE3, SSE4.1, SSE4.2 and POPCNT CPU features (also expose these in virtual machines).
Download `OpenPaint-<version>-ubuntu24.04-amd64.deb` and verify its checksum.
Install from the download directory:

```sh
sudo apt install ./OpenPaint-<version>-ubuntu24.04-amd64.deb
```

OpenPaint appears in the application menu and image Open With choices. The package
does not change the default image viewer. Its Qt libraries are private to the app.
APT may download system dependencies; drawing remains offline.

Install a newer downloaded DEB with the same command to update. Save your work and
close OpenPaint first. No OpenPaint APT repository or automatic update feed is
configured. Remove with `sudo apt remove openpaint`; saved artwork is untouched.
Other distributions should use Flatpak or a tested portable build. A `.deb` suffix
alone does not establish Debian or older Ubuntu compatibility.

## Portable Linux archive

Download the `OpenPaint-<version>-linux-x86_64.tar.gz` asset. Extract it with your
archive manager, then run `openpaint` inside the extracted folder. Keep the whole
folder together; bundled libraries are required. No root access, FUSE or installer.
This targets Ubuntu 24.04 and compatible newer x86_64 Linux desktops.

For example, replacing `<version>` with the downloaded release version:

```sh
tar -xzf OpenPaint-<version>-linux-x86_64.tar.gz
./OpenPaint-<version>-linux-x86_64/openpaint
```

To update, save your work, close OpenPaint and extract a newer release into a new
folder. The sources-and-notices archive is for developers/license compliance, not
installation. `.dmg` files are for macOS, not Linux.

## Historical alpha AppImage

```sh
chmod u+rx OpenPaint-0.1.0-alpha.1-x86_64.AppImage
./OpenPaint-0.1.0-alpha.1-x86_64.AppImage
```

A normal Linux desktop with graphics libraries and a working file portal is
required. If FUSE 2 is unavailable, launch with extraction instead:

```sh
APPIMAGE_EXTRACT_AND_RUN=1 ./OpenPaint-0.1.0-alpha.1-x86_64.AppImage
```

If the runtime reports `Cannot open /proc/self/exe: Permission denied`, ensure
the AppImage has both read and execute permissions using `chmod u+rx` above.
The extraction variable starts with `APPIMAGE`, not `PPIMAGE`. Alternatively use
the portable archive, which does not invoke this runtime.

No root access or installer is required. To update, save your work, close the app,
verify the new download and replace the old AppImage. To uninstall, close it and
remove the AppImage. Images and local custom-color settings remain untouched.

## Flatpak bundle

Install Flatpak using your distribution's normal software manager, then replace
`<version>` with the downloaded version:

```sh
flatpak install --user ./OpenPaint-<version>-x86_64.flatpak
flatpak run io.github.AfshinKI.OpenPaint
```

The first installation obtains the KDE runtime from Flathub and requires an
internet connection. Editing works offline afterward. The app has no network or
broad home-directory permission; open/save dialogs grant selected-file access.
Save and export filenames must include the selected format extension (for example
`drawing.png`). Missing extensions are rejected and the drawing remains unsaved;
retry with the extension included.
This bundle is not a Flathub listing and does not configure an OpenPaint update
feed. Install a newer verified bundle manually to replace it. Stable bundles use
the `stable` branch; old alpha installations use `alpha`. To avoid duplicate menu
entries, uninstall the old alpha branch after saving work:
`flatpak uninstall --user io.github.AfshinKI.OpenPaint//alpha`. Choose one install
format per user so a user Flatpak launcher does not shadow the system DEB.

```sh
flatpak uninstall --user io.github.AfshinKI.OpenPaint
```

Uninstall preserves settings and saved artwork. Runtime removal is a separate
Flatpak operation; do not remove shared runtimes needed by other applications.

## Source and licenses

The combined sources-and-notices archive includes matching OpenPaint source,
bundled third-party sources, dependency inventories and a CycloneDX SBOM. The
portable application also includes license notices. These materials are for
source availability and license compliance; they are not required to run the app.
Older AppImage/Flatpak releases publish these materials as separate assets.

Help → Updates shows the installed version and a manual link to
GitHub Releases. It does not check, download, install or restart automatically.
