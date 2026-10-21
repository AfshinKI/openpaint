# ADR 0008: bounded raster layers and OpenRaster projects

Status: accepted
Date: 2026-10-21

## Context

Layer editing must retain existing tool transactions and save guards. The current
raster tools consume QImage; introducing tiles simultaneously would expand this
feature into a separate renderer/tool rewrite.

## Decision

Keep full premultiplied sRGB rasters for this bounded first implementation: at most
32 layers, 32 MP per canvas, 128 MiB of resident layer pixels and the existing
256 MiB undo budget. Structural history conservatively charges both full stack
snapshots, although Qt shares unchanged pixel buffers. Refuse commands exceeding
that budget. Pixel edits retain rectangular patches addressed by stable layer ID.
Display caches source-over composition; no view owns authoritative pixels.
Tiled large-document support remains a separate measured follow-up.

Use system libarchive for ZIP handling (BSD licensing; distribution notices and
sources are collected by packaging). Only enable ZIP reading, never filesystem
extraction. Bound encoded bytes, decoded entry bytes, entry count, path length,
expansion ratio, XML and layer pixels; reject links, traversal, duplicates,
encryption, nested stacks and unsupported blending. Cancel between entries/chunks.
QXmlStreamReader parses the stack without DTDs/entities. PNG uses the existing
validated decoder. Support full-canvas and contained offset PNG raster layers;
reject off-canvas data rather than discard editable content. Write stored ZIP
entries with mimetype first, stack.xml, raster layers, merged image and thumbnail.
Atomic QSaveFile writes disable direct-write fallback.

Save retains PNG for an ordinary single Background layer; stack/properties require
.ora. Existing .ora backing remains .ora even after merging to one layer. Export
always writes a flattened copy without marking the project clean. Ctrl+Shift+S
remains Save As, with project format available there.

## Alternatives

A bespoke ZIP parser would duplicate complex security-sensitive archive logic.
A new native format would lose the existing OpenRaster interoperability contract.
Full tile storage is deferred until separately benchmarked and scoped.

## Consequences

libarchive-dev is a build dependency. Portable packaging must inventory its runtime
and transitive libraries. The deliberately restricted import subset can reject
valid OpenRaster files; no full-baseline compatibility is claimed. Large multilayer
operations may hit the conservative undo budget before the layer-count limit.

Sources: [OpenRaster layout](https://www.openraster.org/baseline/file-layout-spec.html),
[layer stack](https://www.openraster.org/baseline/layer-stack-spec.html),
[libarchive](https://www.libarchive.org/),
[license](https://github.com/libarchive/libarchive/blob/master/COPYING).
See [acceptance plan](../plans/0058-layers.md).
