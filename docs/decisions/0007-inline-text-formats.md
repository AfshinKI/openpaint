# ADR 0007: preserve inline font runs through raster commit

Status: accepted
Date: 2026-10-20

## Context

The text box used one global font for both Qt Quick input and C++ rasterization.
Changing fonts therefore restyled existing text, and saving only the plain draft
could not preserve mixed fonts.

## Decision

Keep TextEdit in PlainText mode for input and clipboard safety. Its transient
QTextDocument holds character formats. Range changes merge only requested font
properties through QTextCursor. Caret-only changes retain a pending insertion font;
native keyboard/paste/input-method events are forwarded within a QTextCursor edit
block, and newly inserted characters receive the font before that block closes.
Local undo/redo uses the transient document with a bounded cursor. Cursor
navigation/selection and undo/redo clear that pending format. Qt Quick's
cursorSelection supplies the current font but does not apply caret-only changes:
its setter can automatically restyle the surrounding word. The editor keeps a guarded,
non-owning reference to the transient document and clones it for C++ raster previews.
The document is laid out in canvas pixels; the view scales with zoom.

The existing Document preview transaction owns pixels and commits one content
history command. Text remains rasterized on commit, without persistent text layers.
Headless plain-text rendering remains supported. Alignment and colors stay box-wide.

## Alternatives

HTML serialization would add parsing, escaping and resource concerns unnecessarily.
A second custom rich-text editor would duplicate Qt's selection, input-method and
local undo machinery. Storing only a single font cannot satisfy the requested flow.

## Consequences

Qt's cursorSelection API is available since 6.7, within the existing 6.8 minimum,
but is still marked preliminary/technology preview; behavior is covered by shell
regressions. Font availability remains machine-dependent. No network source or
HTML is loaded, and no new dependency or saved-file migration is needed.

Sources: [Qt TextEdit](https://doc.qt.io/qt-6/qml-qtquick-textedit.html),
[Qt text editor example](https://doc.qt.io/qt-6/qtquickcontrols-texteditor-example.html),
and [QTextCharFormat](https://doc.qt.io/qt-6/qtextcharformat.html).
Validation is recorded in [plan 0056](../plans/0056-text-range-formatting.md).
