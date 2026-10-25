# Project website

The OpenPaint product page is plain HTML/CSS, licensed under GPL-3.0-or-later.
It uses the original app icon and generated project screenshots. Image provenance
is documented in [the asset notes](../docs/assets/README.md). The three stroke
illustrations are original SVG examples, not captured brush output.

## Preview

Run `python3 tools/build_site.py` from the repository root, then
`python3 -m http.server 8000 --directory build/website` and open the local server.
There are no package downloads, external fonts, JavaScript, cookies or analytics.

## GitHub Pages

For a new repository or a reset Pages configuration, select **Settings → Pages → Build and deployment →
Source → GitHub Actions** in the repository. The Project website workflow stages
only the page, selected assets and license; it does not publish the repository
or documentation directory. PRs build but cannot deploy. Main-branch updates to
website inputs deploy through the `github-pages` environment. Restrict that
environment to main in repository settings.

If Pages was not enabled when the main workflow ran, enable it and manually run
Project website on main. The project deployment URL is
<https://afshinki.github.io/openpaint/>; verify the successful deployment before
advertising a new deployment. The v2.1 deployment succeeded and the public page was visually checked at desktop
and 390×844 mobile viewport sizes; see [release evidence](../docs/plans/0080-release-2.1.0.md).
Repository settings are not changed by adding the workflow.

Download links point to the latest GitHub release rather than fixed filenames.
Update page claims when the shipped feature scope changes. Check desktop and
narrow layouts, FAQ keyboard behavior, and relative asset paths before merging.

## Release maintenance

Follow the [release checklist](../docs/RELEASES.md#website-and-readme-release-checklist).
`release.json` records the application version, source commit and screenshot set.
The build rejects a version mismatch with CMake; repository CI runs that check.
Capture filenames identify the verified source release version. Clearly distinguish isolated release-build harness captures from installed-executable captures. Record the
exact rendering conditions in the asset provenance notes; use glass enabled at
40% surface opacity and a staged original backdrop in light and dark mode.
Crop away the surrounding desktop and use transparent rounded outer corners.
Keep plain copy, rounded controls and soft translucent surfaces; preserve contrast
and visible keyboard focus. App screenshots remain real captures, not mockups.
A matching version number is not proof of accurate images: inspect every capture.
