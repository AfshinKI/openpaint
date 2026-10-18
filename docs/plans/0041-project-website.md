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
