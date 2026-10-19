# Feature: designer attribution in Updates

Status: implemented; local checks passed; PR deferred
Feature IDs: F25

## Acceptance

Updates displays “Designer: Afshin Kashani, PhD, P.Eng” beneath the application
version, in theme-aware secondary text that wraps at narrow widths. This is the
maintainer-requested public attribution, with no contact information or new links.
Update behavior, artwork and saved settings are unchanged.

## Validation

Qt 6.12 dev build, updatesWorkflow, repository and whitespace checks passed.
Inspected generated light/dark Updates captures at 1024×640; attribution and
controls fit. Next: later PR/package validation.
