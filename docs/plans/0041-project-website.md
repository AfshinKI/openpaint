# Feature: OpenPaint project website

Status: in progress
Scope: public project information and GitHub Pages hosting; no editor behavior change.

## User outcome

Visitors can see the real OpenPaint interface, understand its shipped tools and
limits, and find Linux downloads and source. The website uses the existing app
icon, plain descriptions and a restrained product-page layout.

## Behavior and implementation

Static HTML/CSS with native FAQ disclosure controls. No external fonts, scripts,
analytics, accounts or contact forms. Links lead to the public repository and
release downloads. Original project screenshots and icons retain their GPL license.
A small build script stages only website files and selected project assets.
GitHub Actions publishes the staged directory from reviewed main commits only.
The existing ChatGPT-hosted draft is not updated by this change.

## Acceptance cases

- [x] Real app icon and screenshots appear; planned features are labeled as absent.
- [x] Download and source links work under the GitHub Pages project subpath.
- [x] Desktop and narrow layouts fit; keyboard navigation and FAQ controls work.
- [x] Repository checks and static asset/link validation pass.
- [ ] A focused PR passes protected checks before merge.
- [ ] GitHub Pages is enabled and the public endpoint is verified.

## Validation and handoff

Validation on 2026-10-18: website staging and repository checks pass; all 26
validation-tool tests pass. Static HTML links/fragments and project-relative
assets resolve. Browser checks at desktop size and 390px viewport confirm loaded
images, no horizontal overflow, and keyboard FAQ expansion. Existing project
screenshots contain generated test content. Native application tests were not
rerun because no application behavior changed.

Next: protected PR checks, merge, then Pages activation and endpoint verification.
Repository Pages settings are separate from committing the workflow. No public
deployment is claimed until verified.

## Release screenshot refresh

The maintainer requested installed v1.2.0 light/dark glass screenshots and approved
local capture utilities. Four reviewed images now come from installed DEB 1.2.0-1,
with glass enabled and Surface opacity 40%, over an original illustrated backdrop.
No personal desktop content was saved. A separate app instance used isolated
preferences; original artwork and settings remained untouched. Capture provenance
and the original-art generator are included. Website and README use these images.

Release policy now requires website/README review, versioned images, 40% glass
captures and post-publication verification. Repository CI checks website release
metadata against CMake. A regression test proves stale metadata fails. All 27
Python tests and formatting checks pass. The optional source screenshot harness
builds and its targeted offscreen test passes (3 QtTest phases); those opaque test
renders are separate from the installed glass screenshots used for publication.

The maintainer explicitly authorizes merging once all checks pass. Next: verify
CI on the updated PR head, merge through branch protection, and activate Pages.

## Rounded glass presentation

The maintainer requested softer, rounded website surfaces and app-only imagery.
The page now uses pill controls, rounded translucent panels and subtle tinted
backgrounds. All four installed captures are cropped to the app bounds with
22px transparent rounded corners; captured UI and glass pixels are preserved.
Acceptance: no surrounding desktop border, rounded image corners in README and
website, readable glass surfaces, working FAQ keyboard controls, and no narrow
screen overflow. Recheck these plus repository/site checks before merging.

Validation: site build, repository validation and all 27 tooling tests pass.
Desktop and 390px browser review confirms cropped rounded images, readable
surfaces, no horizontal overflow and keyboard FAQ expansion. PNG alpha corners
and empty metadata are verified for all four images. Updated CI is pending.
