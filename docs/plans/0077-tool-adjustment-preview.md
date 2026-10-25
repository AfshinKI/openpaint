# Tool adjustment preview

Status: implemented; focused combined-preview validation passed
Target: v2.1.0

Show a view-only circular size/color/opacity sample on the visible canvas while
adjusting the floating controls. Respect zoom, clip to visible artwork, hide after
adjustment, and leave document pixels and dirty state unchanged.

Acceptance evidence: keyboardControlsAndAccessibility passed in the combined
preview, checking diameter, color, opacity, appearance/disappearance and unchanged
pixels/dirty state. Build and zero-warning QML lint passed. The preview is a circle,
not a simulation of textured brushes. Next: review the isolated change for merge.
