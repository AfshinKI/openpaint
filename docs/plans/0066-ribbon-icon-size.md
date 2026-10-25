# Feature: larger ribbon tool icons

Status: acceptance defined; native validation deferred
Feature IDs: F01
Target: v2.1.0

## Outcome and acceptance

Increase the visible Image/Tools glyphs to 28 logical pixels (from 20 monochrome
or 24 colored) within their existing 32-pixel targets. Increase Selection to 28
and Brushes to 32 within its larger target. Keep menus/footer/shape gallery sizing
unchanged. Preserve dropdown targets, all keyboard actions, two-row alignment,
minimum 1024×640 bounds and both themes. Artwork/history are unaffected.

Existing coloredRibbonWorkflow and frostedToolSelection cover minimum-size bounds,
keyboard activation, tool colors and theme captures. Extend bounds checks for the
larger content. Application build, QML lint and UI execution are deferred to the
final validation pass per maintainer request; no visual verification claimed yet.
