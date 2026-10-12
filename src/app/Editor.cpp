// SPDX-License-Identifier: GPL-3.0-or-later
#include "Editor.h"
void Editor::begin(QPointF point, bool secondaryButton) {
    if (!image().rect().contains(point.toPoint()))
        return;
    if (document.beginStroke(point, eraser ? secondary : (secondaryButton ? secondary : primary),
                             brushSize, eraser && transparentEraser)) {
        emit pixelsChanged(QRect());
        emit changed();
    }
}
void Editor::extend(QPointF point) {
    const auto rect = document.extendStroke(point);
    if (!rect.isEmpty())
        emit pixelsChanged(rect);
}
void Editor::finish() {
    if (!document.active())
        return;
    document.commitStroke();
    emit pixelsChanged(QRect());
    emit changed();
}
void Editor::cancel() {
    document.cancelStroke();
    emit pixelsChanged(QRect());
    emit changed();
}
void Editor::undo() {
    finish();
    if (document.undo()) {
        emit pixelsChanged(QRect());
        emit changed();
    }
}
void Editor::redo() {
    finish();
    if (document.redo()) {
        emit pixelsChanged(QRect());
        emit changed();
    }
}
