# Feature: complete M1 PNG import and failure validation

Status: implemented; protected merge requires CI success
Feature IDs: F15, F16, F26
Milestone: M1

## Outcome and scope

Import static PNG with all eight TIFF/EXIF orientations normalized to pixels,
convert colors to sRGB, and discard personal metadata. Reject corrupt chunks and
ambiguous orientation instead of displaying or saving the wrong artwork. Retain
atomic replacement and worker snapshot semantics. No new file formats or UI tools.

## Implementation

Read only the bounded TIFF IFD0 orientation field; never traverse arbitrary EXIF
subdirectories. Strip EXIF before Qt decoding so orientation is applied exactly
once. Validate PNG chunk CRC with system zlib (the PNG CRC algorithm), structure,
and encoded limits before decoding. Keep metadata and parser bytes in the worker.
Use an injectable encoder alongside the existing commit injection to exercise
short writes and failures without filling the user's disk. Production uses QImageWriter and QSaveFile with direct-write fallback disabled.
A forwarding device records failed/short writes but lets the codec unwind normally;
Qt 6.8.3 otherwise leaks its ICC temporary on the libpng longjmp error path.
Encoding failure always cancels the temporary file. ICC profiles must decode to a
valid Qt color space within 4 MiB; malformed/unsupported profiles are rejected.
System zlib is dynamically linked under the [zlib license](https://zlib.net/zlib_license.html);
no third-party source is copied.

## Acceptance

- [x] Both TIFF byte orders and all eight orientations give exact expected pixels.
- [x] Missing orientation means identity; invalid offsets/types/counts/values and
      duplicate EXIF/orientation tags are rejected.
- [x] Corrupt CRC, invalid chunk order, truncation and oversized inputs are rejected.
- [x] Text and EXIF never survive export; sRGB and alpha round-trip remain correct.
- [x] Encoder failure, partial writes, failed commit, read-only destination and
      cancellation preserve the original bytes; snapshot edits remain dirty.
- [x] Targeted tests, full suites, sanitizers, formatting and repository checks pass.

## Evidence and handoff

Qt 6.8.3 / Ubuntu 24.04: all 26 PNG Qt Test results pass, including 16
orientation rows. All seven Debug and ASan/UBSan CTest suites pass with leak
detection enabled. Formatting, QML lint, repository checks and seven tooling
tests pass. The initial fault test exposed a 498-byte Qt ICC allocation leak;
the forwarding device eliminates it without sanitizer exceptions. Disk-full is
a deterministic short-write device simulation, not a physical full-disk claim.

Next: M1 keyboard/desktop workflows and
performance baseline, then milestone closure against the original roadmap gate.
Full portal packaging, stylus, Wayland/KDE and Orca certification remain later gates.

Format reference: [W3C PNG specification](https://www.w3.org/TR/png-3/#eXIf).
