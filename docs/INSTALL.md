# OpenPaint alpha installation

Download assets from [GitHub Releases](https://github.com/AfshinKI/openpaint/releases).

Linux x86_64. Tested release environments and artifact digests are recorded in the
release evidence. This is an early alpha; keep copies of important originals.

## Verify downloads

Download `SHA256SUMS` and the desired assets from the same GitHub release. In that
directory, run `sha256sum --ignore-missing -c SHA256SUMS`. Every downloaded entry
must report OK. Checksums detect changed bytes; these alpha artifacts do not have
an OpenPaint signing signature or an automatic updater.

## Recommended: portable Linux archive

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

## Portable AppImage

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

## Flatpak test bundle

Install Flatpak using your distribution's normal software manager, then:

```sh
flatpak install --user ./OpenPaint-0.1.0-alpha.1-x86_64.flatpak
flatpak run io.github.AfshinKI.OpenPaint
```

The first installation obtains the KDE runtime from Flathub and requires an
internet connection. Editing works offline afterward. The app has no network or
broad home-directory permission; open/save dialogs grant selected-file access.
This bundle is not a Flathub listing and does not configure an OpenPaint update
feed. Install a newer verified bundle manually to replace it.

```sh
flatpak uninstall --user io.github.AfshinKI.OpenPaint
```

Uninstall preserves settings and saved artwork. Runtime removal is a separate
Flatpak operation; do not remove shared runtimes needed by other applications.

## Source and licenses

The matching OpenPaint source archive contains the CMake build, packaging recipes,
GPL license, contribution rules and documentation. The AppImage includes notices;
the separate third-party source archive supplies corresponding bundled-library
sources and runtime build references. Dependency inventories and a CycloneDX SBOM
accompany the release. The Flatpak's shared KDE runtime is distributed separately
by Flathub; its exact tested commit and source recipe are in the Flatpak inventory.

Help → Updates shows the installed version and a manual link to GitHub Releases.
It does not check, download, install or restart automatically.
